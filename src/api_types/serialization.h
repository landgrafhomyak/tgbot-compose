#ifndef API_TYPES_GENERATOR_H
# define API_TYPES_GENERATOR_H


# ifdef __cplusplus
#  include <cstdint>
# else
#  include <stdint.h>
# endif

# ifndef NO_PYTHON
#  include <Python.h>
# endif

static inline uint64_t hash_string(const char *key)
{
    uint64_t h = 525201411107845655ull;
    for (; *key; key++)
    {
        h ^= *key;
        h *= 0x5bd1e9955bd1e995;
        h ^= h >> 47;
    }
    return h;
}

# ifndef NO_PYTHON

static inline PyObject *parse_kwargs(PyObject *args, PyObject *kwargs)
{
    PyObject *data = NULL;
    if (kwargs == NULL)
    { goto FROM_ARGS; }

    if (!PyDict_Check(kwargs))
    {
        PyErr_Format(PyExc_TypeError, "**kwargs must be dict or its subclass");
        return NULL;
    }

    if (PyDict_GET_SIZE(kwargs) == 0)
    { goto FROM_ARGS; }

    FROM_KWARGS:
    data = kwargs;
    goto VALIDATE;

    FROM_ARGS:
    if (!PyTuple_Check(args))
    {
        PyErr_Format(PyExc_TypeError, "*args must be tuple or its subclass");
        return NULL;
    }

    if (PyTuple_GET_SIZE(args) > 1)
    {
        PyErr_Format(PyExc_TypeError, "Unexpected positional arguments in tgbot api type constructor, must be only one (json serialized object (dict)) or keyword arguments with data");
        return NULL;
    }

    if (PyTuple_GET_SIZE(args) < 1)
    {
        PyErr_Format(PyExc_TypeError, "Not enough arguments in tgbot api type constructor, must be only one (json serialized object (dict)) or keyword arguments with data");
        return NULL;
    }

    data = PyTuple_GET_ITEM(args, 0);
    if (!PyDict_Check(args))
    {
        PyErr_Format(PyExc_TypeError, "JSON serialized object must be a dict");
        return NULL;
    }
    VALIDATE:

    if (!PyArg_ValidateKeywordArguments(data))
    {
        return NULL;
    }

    return data;
}

# endif
#endif /* API_TYPES_GENERATOR_H */
