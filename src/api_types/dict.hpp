#ifndef API_TYPES_DICT_HPP
# define API_TYPES_DICT_HPP

# include <cstdint>
# include <cstring>
# include <cstddef>

# include <utility>
# include <exception>

extern "C" {
# include <Python.h>
};

# include "macro.h"

//template<typename object>
class DictMeta
{
public:
    using HashInt = uint64_t;
//    using Object = object;

    static inline constexpr DictMeta::HashInt hash_string(const char *key) noexcept
    {
        DictMeta::HashInt h = 525201411107845655ull;
        for (; *key; ++key)
        {
            h ^= *key;
            h *= 0x5bd1e9955bd1e995;
            h ^= h >> 47;
        }
        return h;
    }

    PyTypeObject const *const owner;
    DictMeta const *const base;

private:
    struct Node
    {
        DictMeta::HashInt hash;
        char const *key = nullptr;
        size_t offset = (size_t) -1;

        PyObject *(*default_value_getter)() = nullptr;

        PyTypeObject const* required_type;

        inline PyObject *&extract(PyObject *object) const noexcept {
            return *(PyObject **)(((uintptr_t)object) + this->offset);
        }
    };

public:
    static inline constexpr const size_t MAX_TABLE_SIZE = 20;
    const size_t fields_count = 0;

private:
    DictMeta::Node table[DictMeta::MAX_TABLE_SIZE]{};

    static void hash_collision() noexcept
    {}

    static void table_overflow() noexcept
    {}

    static inline constexpr int strcmp(const char *a, const char *b) noexcept
    {
        int diff = 0;
        do
        {
            if ((diff = *a - *b) != 0)
            {
                return diff;
            }
        } while (*a++ != 0 && *b++ != 0);
        return 0;
    }

public:
    struct InputEntry
    {
        char const *name;
        size_t offset;
        PyTypeObject &required_type;

        PyObject *(*default_value_getter)();
    };

    template<typename ...T>
    constexpr DictMeta(
            PyTypeObject const &owner,
            DictMeta const *const base,
            T &&... table
    ) noexcept:
            owner(&owner),
            base(base)
    {
        auto nodes = (DictMeta::Node *) this->table;
        size_t i = 0, j = 0;
        DictMeta::HashInt hash = 0;

        if (base != nullptr)
        {
            for (; i < base->fields_count; i++)
            {
                nodes[i] = base->table[i];
            }
        }

        for (auto entry: {table...})
        {
            hash = DictMeta::hash_string(entry.name);
            for (j = 0; j < i; j++)
            {
                if (hash == nodes[j].hash)
                {
                    if (DictMeta::strcmp(entry.name, nodes[j].key) != 0)
                    {
                        hash_collision();
//                        throw std::exception("Hash collision");
                    }
                    nodes[j].offset = entry.offset;
                    nodes[j].required_type = &(entry.required_type);
                    nodes[j].default_value_getter = entry.default_value_getter;
                    break;
                }
            }
            if (j == i)
            {
                if (i >= DictMeta::MAX_TABLE_SIZE)
                {
//                    throw std::exception("Table overflow");
                    table_overflow();
                }
                nodes[i++] = DictMeta::Node{
                        .hash = DictMeta::hash_string(entry.name),
                        .key = entry.name,
                        .offset= entry.offset,
                        .default_value_getter = entry.default_value_getter,
                        .required_type = &(entry.required_type),
                };
                continue;
            }

            *(size_t *) &(this->fields_count) = i;
            for (i = 0; i < this->fields_count; i++)
            {
                for (j = 0; j < i; j++)
                {
                    if (nodes[i].hash < nodes[j].hash)
                    {
                        std::swap(nodes[i], nodes[j]);
                    }
                }
            }
        }
    }

    constexpr inline DictMeta::Node const *operator[](const char *key) const noexcept
    {
        auto hash = DictMeta::hash_string(key);
        size_t lo = 0, hi = this->fields_count, mi = 0;
        while (lo < hi)
        {
            mi = (lo + hi) / 2;
            if (this->table[mi].hash < hash)
            {
                lo = mi + 1;
            }
            else
            {
                hi = mi;
            }
        }
        return this->table[lo].hash == hash ? &(this->table[lo]) : nullptr;
    }

private:
    inline DictMeta::Node const *get_node(PyObject *key) const noexcept
    {
        char const *key_s = PyUnicode_AsUTF8((PyObject *) key);
        if (key_s == nullptr)
        {
            return nullptr;
        }

        return (*this)[key_s];
    }

public:
    inline PyObject *get_item_i(PyObject *self, PyObject *key) const noexcept
    {
        DictMeta::Node const *node;
        PyObject *value;
        if (!PyUnicode_Check(key))
        {
            PyErr_Format(PyExc_TypeError, "ApiType key must be string, not `%s`", Py_TYPE(key)->tp_name);
            return nullptr;
        }

        node = this->get_node(key);
        if (node == nullptr || PyUnicode_CompareWithASCIIString(key, node->key) != 0)
        {
            PyErr_Format(PyExc_KeyError, "%U", key);
            return nullptr;
        }

        value = (PyObject *) (((uintptr_t) self) + node->offset);
        if (value == nullptr)
        {
            Py_RETURN_NONE;
        }
        return Py_NewRef(value);
    }


    static inline PyObject *ParseArgKwargs(PyObject *args, PyObject *kwargs) noexcept
    {
        PyObject *data = nullptr;
        if (kwargs == nullptr)
        {
            goto FROM_ARGS;
        }

        if (!PyDict_Check(kwargs))
        {
            PyErr_Format(PyExc_TypeError, "**kwargs must be dict or its subclass");
            return nullptr;
        }

        if (PyDict_GET_SIZE(kwargs) == 0)
        {
            goto FROM_ARGS;
        }

        FROM_KWARGS:
        data = kwargs;
        goto VALIDATE;

        FROM_ARGS:
        if (!PyTuple_Check(args))
        {
            PyErr_Format(PyExc_TypeError, "*args must be tuple or its subclass");
            return nullptr;
        }

        if (PyTuple_GET_SIZE(args) > 1)
        {
            PyErr_Format(PyExc_TypeError, "Unexpected positional arguments in tgbot api type constructor, must be only one (json serialized object (dict)) or keyword arguments with data");
            return nullptr;
        }

        if (PyTuple_GET_SIZE(args) < 1)
        {
            PyErr_Format(PyExc_TypeError, "Not enough arguments in tgbot api type constructor, must be only one (json serialized object (dict)) or keyword arguments with data");
            return nullptr;
        }

        data = PyTuple_GET_ITEM(args, 0);
        if (!PyDict_Check(args))
        {
            PyErr_Format(PyExc_TypeError, "JSON serialized object must be a dict");
            return nullptr;
        }
        VALIDATE:

        if (PyArg_ValidateKeywordArguments(data))
        {
            return nullptr;
        }

        return data;
    }

    template<class object_t>
    inline object_t *ConstructObject(PyObject *data, Py_ssize_t ob_length)
    {
        size_t i = 0;
        PyObject *existing_keys = nullptr;
        PyObject *key;
        PyObject *value;
        PyObject *node;
        object_t *self;

        ifNullRet(existing_keys = PySet_New(data));

        /* todo remove ignores */

        for (i = 0; i < this->fields_count; i++)
        {
            ifNull(key = PyUnicode_FromString(this->table[i].key))
            {
                Py_DECREF(existing_keys);
                return nullptr;
            }
            if (PySet_Discard(existing_keys, key) == -1)
            {
                Py_DECREF(key);
                Py_DECREF(existing_keys);
                return nullptr;
            }
            Py_DECREF(key);
        }

        if (PySet_GET_SIZE(existing_keys) > 0)
        {
            PyErr_Format(PyExc_AttributeError, "Unexpected members in object `%s`: %S", this->owner->tp_name, existing_keys);
            Py_DECREF(existing_keys);
            return nullptr;
        }
        Py_DECREF(existing_keys);

        ifNull(self = this->owner->tp_alloc((PyTypeObject *) this->owner, ob_length))
        {
            PyErr_NoMemory();
            return NULL;
        }
        for (i = 0; i < this->fields_count; i++)
        {
            value = PyDict_GetItemString(data, this->table[i].key);
            if (value == nullptr)
            {
                Py_DECREF(self);
                if (this->table[i].default_value_getter == nullptr) {
                    PyErr_Format(PyExc_AttributeError, "Missed required member '%s'", this->table->key);
                    return nullptr;
                }
                this->table[i].extract(self) = this->table[i].default_value_getter();
                continue;
            }

            if (!PyType_IsSubtype(Py_TYPE(value), (PyTypeObject *)(this->table[i].required_type)))
            {
                Py_DECREF(self);
                PyErr_Format(PyExc_TypeError, "Member '%s' must be `%s` or its subclass, but `%s` got", this->table[i].key, this->table[i].required_type->tp_name, Py_TYPE(value)->tp_name);
                return nullptr;
            }
            Py_INCREF(value);
            this->table[i].extract(self) = value;
        }

        return self;
    }


};

#define prop(NAME, OBJECT, REQUIRED_TYPE, DEFAULT_VALUE) (DictMeta::InputEntry{#NAME, offsetof(OBJECT, NAME), (REQUIRED_TYPE), (DEFAULT_VALUE)})

static PyObject *DictMeta_GetNone()
{
    Py_RETURN_NONE;
}

#define def(NAME, REQUIRED_TYPE, GETTER) (DictMeta::InputEntry{#NAME, (size_t)-1, (REQUIRED_TYPE), (GETTER)})


#endif /* API_TYPES_DICT_HPP */
