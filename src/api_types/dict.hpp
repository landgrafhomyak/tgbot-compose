#ifndef API_TYPES_DICT_HPP
# define API_TYPES_DICT_HPP

# include <cstdint>

# include <Python.h>

class DictMeta
{
public:
    using HashInt = uint64_t;

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

    PyTypeObject const *const owner = nullptr;
    DictMeta const *const base = nullptr;

private:
    struct DictMeta_Node
    {
        DictMeta::HashInt hash;
        char const *key = nullptr;
        size_t offset = (size_t) -1;
        PyObject *default_value = nullptr;
    };

public:
    static inline constinit const size_t MAX_TABLE_SIZE = 20;
    const size_t fields_count = 0;

private:
    DictMeta::DictMeta_Node const table[DictMeta::MAX_TABLE_SIZE]{};


public:
    struct DictMeta_InputEntry
    {
        char const *name;
        size_t offset;
        PyObject *default_value;
    };

public:

    consteval inline DictMeta(
            PyTypeObject const &owner,
            DictMeta const &base,
            std::initializer_list<DictMeta::DictMeta_InputEntry> table
    )
    {
        *(PyTypeObject const **) &this->owner = &owner;
        *(DictMeta const **) &this->base = &base;

        auto nodes = (DictMeta::DictMeta_Node *) this->table;
        size_t i, j;


        for (i = 0; i < base.fields_count; i++)
        {
            nodes[i] = base.table[i];
        }

        DictMeta::HashInt hash;
        for (auto entry: table)
        {
            hash = DictMeta::hash_string(entry.name);
            for (j = 0; j < i; j++)
            {
                if (hash == nodes[hash].hash)
                {
                    if (strcmp(entry.name, nodes[j].key) != 0)
                    {
                        throw std::exception("Hash collision");
                    }
                    nodes[j].offset = entry.offset;
                    nodes[j].default_value = entry.default_value;
                    break;
                }
            }
            if (j == i)
            {
                if (i >= DictMeta::MAX_TABLE_SIZE)
                {
                    throw std::exception("Table overflow");
                }
                nodes[i++] = DictMeta::DictMeta_Node{
                        .hash = DictMeta::hash_string(entry.name),
                        .key = entry.name,
                        .offset= entry.offset,
                        .default_value = entry.default_value
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

    constexpr inline DictMeta::DictMeta_Node const *operator[](const char *key) const noexcept
    {
        auto hash = DictMeta::hash_string(key);
        size_t lo = 0, hi = this->fields_count, mi;
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
    inline DictMeta_Node const *get_node(PyUnicodeObject *key) const noexcept
    {
        char const *key_s = PyUnicode_AsUTF8((PyObject *) key);
        if (key_s == nullptr)
        {
            return nullptr;
        }

        return (*this)[key_s];
    }


};


#endif /* API_TYPES_DICT_HPP */
