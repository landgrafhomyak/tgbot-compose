#ifndef API_TYPES_PROPERTY_H
# define API_TYPES_PROPERTY_H

# include <Python.h>

typedef struct Property_Object
{
    PyObject_HEAD
    char const *const name;
    PyTypeObject *const value_type;
    PyTypeObject *const instance_type;
    struct
    {
        int mutable: 1;
        int optional: 1;
        size_t offset: sizeof(size_t) * 8 - 2;
    };
    struct Property_Object const *const kwargs;
} Property_Object;


extern PyTypeObject Property_Type;

# define Property_SInit(NAME, IS_WRITABLE, IS_OPTIONAL, OFFSET, KWARGS) \
    {PyObject_HEAD_INIT(NULL) (NAME), NULL, NULL, {(IS_WRITABLE), (IS_OPTIONAL), (OFFSET)}, (KWARGS)}

# define Property_ListEnd {.name = NULL}

# define Property_RInit(INSTANCE_TYPE_SCALAR, ARRAY, INDEX, VALUE_TYPE_SCALAR)           \
    {                                                                                    \
       Py_SET_TYPE(&(ARRAY)[(INDEX)], &Property_Type);                                   \
       *(PyTypeObject **)&((ARRAY)[(INDEX)].instance_type) = &(INSTANCE_TYPE_SCALAR);    \
       *(PyTypeObject **)&((ARRAY)[(INDEX)].value_type) = &(VALUE_TYPE_SCALAR);          \
    } ((void)0)

PyObject *Property_Get(Property_Object const *self, PyObject *instance, PyObject *owner);
int Property_SetNoCheck(Property_Object const *self, PyObject *instance, PyObject *value);
int Property_Set(Property_Object const *self, PyObject *instance, PyObject *value);

PyObject *Parse_ArgsKwargsToDict(PyObject *args, PyObject *kwargs);

PyObject *Parse_DictToProperties(
        PyObject *data,
        char const *const *ignore_keywords,
        PyTypeObject *cls,
        Py_ssize_t ob_length,
        size_t meta_quantity,
        ...
);

int Property_StoreToClass(PyTypeObject *cls, Property_Object *array);

# define clsProps(CLASS_SCALAR, ARRAY) if ((Property_StoreToClass(&(CLASS_SCALAR), (ARRAY)))) { return 1; } ((void)0)

//# define addJson(DESTINATION, )

#endif /* API_TYPES_PROPERTY_H */
