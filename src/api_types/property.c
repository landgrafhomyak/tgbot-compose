#include <Python.h>
#include <structmember.h>
#include "macro.h"
#include "__init__.h"
#include "property.h"

#if 0
typedef struct RoGetAttrProperty_Object
{
    PyObject_HEAD
    PyTypeObject *const required_type;
    PyObject *const name;
} RoGetAttrProperty_Object;

static PyObject *RoGetAttrProperty_Call(RoGetAttrProperty_Object *self, PyObject *args, PyObject *kwargs);

static PyObject *RoGetAttrProperty_Get(RoGetAttrProperty_Object *self, PyObject *instance, PyObject *owner);

static void RoGetAttrProperty_Dealloc(RoGetAttrProperty_Object *self);

static PyMemberDef RoGetAttrProperty_Members[] = {
        {"__name__",      T_OBJECT_EX, offsetof(RoGetAttrProperty_Object, name),          READONLY},
        {"required_type", T_OBJECT_EX, offsetof(RoGetAttrProperty_Object, required_type), READONLY},
        {NULL}
};

static PyTypeObject RoGetAttrProperty_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types._ro_getattr_property",
        .tp_basicsize = sizeof(RoGetAttrProperty_Object),
        .tp_call = (ternaryfunc) RoGetAttrProperty_Call,
        .tp_descr_get = (descrgetfunc) RoGetAttrProperty_Get,
        .tp_dealloc = (destructor) RoGetAttrProperty_Dealloc
};
#endif

static PyObject *Property_Call(Property_Object *self, PyObject *args, PyObject *kwargs);

static PyObject *Property_GetOffset(Property_Object *self, void *closure);

static PyObject *Property_GetIsOptional(Property_Object *self, void *closure);

static PyObject *Property_GetIsMutable(Property_Object *self, void *closure);

static void Property_Dealloc(Property_Object *self);

PyMemberDef Property_Members[] = {
        {"__name__",      T_STRING,    offsetof(Property_Object, name),          READONLY},
        {"value_type",    T_OBJECT_EX, offsetof(Property_Object, value_type),    READONLY},
        {"instance_type", T_OBJECT_EX, offsetof(Property_Object, instance_type), READONLY},
        {NULL}
};

PyGetSetDef Property_GetSet[] = {
        {"offset",      (getter) Property_GetOffset,     NULL},
        {"is_optional", (getter) Property_GetIsOptional, NULL},
        {"is_mutable",  (getter) Property_GetIsMutable,  NULL},
        {NULL}
};

PyTypeObject Property_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types._property",
//        .tp_basicsize = sizeof(Property_Object),
//        .tp_call = (ternaryfunc) Property_Call,
//        .tp_descr_get = (descrgetfunc) Property_Get,
//        .tp_descr_set = (descrsetfunc) Property_Set,
//        .tp_members = Property_Members,
//        .tp_getset = Property_GetSet,
//        .tp_dealloc = (destructor) Property_Dealloc
};

#if 0
static PyObject *RoGetAttrProperty_Call(RoGetAttrProperty_Object *self, PyObject *args, PyObject *kwargs)
{
    static char const *const kw_names[] = {"", NULL};
    PyObject *instance;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", (char **) kw_names, &instance))
    {
        return NULL;
    }
    return RoGetAttrProperty_Get(self, instance, NULL);
}

static PyObject *RoGetAttrProperty_Get(RoGetAttrProperty_Object *self, PyObject *instance, PyObject *owner)
{
    if (!PyType_IsSubtype(Py_TYPE(instance), self->required_type))
    {
        PyErr_Format(PyExc_TypeError, "Can't extract attribute `%s` from %R, must be %R or it subclass", self->name, Py_TYPE(instance), self->required_type);
        return NULL;
    }
    return PyObject_GetAttr(instance, self->name);
}

static void RoGetAttrProperty_Dealloc(RoGetAttrProperty_Object *self)
{}
#endif

static PyObject *Property_Call(Property_Object *self, PyObject *args, PyObject *kwargs)
{
    static char const *const kw_names[] = {"instance", "value", NULL};
    PyObject *instance = NULL;
    PyObject *value = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", (char **) kw_names, &instance, &value))
    {
        return NULL;
    }

    ifNull(value)
    {
        return Property_Get(self, instance, NULL);
    }
    else
    {
        switch (Property_Set(self, instance, value))
        {
            case 0:
                Py_RETURN_NONE;
            case -1:
                return NULL;
            default:
                Py_UNREACHABLE();
        }
    }
}

#define field (*(PyObject **) ((uintptr_t) (instance) + self->offset))

PyObject *Property_Get(Property_Object const *self, PyObject *instance, PyObject *owner)
{
    PyObject *value;

    if (!PyType_IsSubtype(Py_TYPE(instance), self->instance_type))
    {
        PyErr_Format(PyExc_TypeError, "Can't get member `%s` from `%s`, must be `%s` or its subclasses", self->name, Py_TYPE(instance)->tp_name, self->instance_type->tp_name);
        return NULL;
    }

    ifNull(value = field)
    {
        if (!self->optional)
        {
            PyErr_Format(PyExc_RuntimeError, "Required field `%s` of `%s` is set to None", self->name, self->instance_type->tp_name);
            return NULL;
        }
        Py_RETURN_NONE;
    }

    if (!PyType_IsSubtype(Py_TYPE(value), self->value_type))
    {
        PyErr_Format(PyExc_RuntimeError, "Field `%s` of `%s` has wrong type `%s`, must be `%s` or its subclasses", self->name, self->instance_type->tp_name, Py_TYPE(value)->tp_name, self->value_type->tp_name);
        return NULL;
    }

    Py_INCREF(value);
    return value;
}

int Property_SetNoCheck(Property_Object const *self, PyObject *instance, PyObject *value) {
    if (value == NULL || value == Py_None)
    {
        if (!self->optional && self->value_type != Py_TYPE(Py_None))
        {
            PyErr_Format(PyExc_TypeError, "Field `%s` of `%s` is not optional and can't be set to None or deleted", self->name, self->instance_type->tp_name);
            return -1;
        }
        field = NULL;
        return 0;
    }

    if (!PyType_IsSubtype(Py_TYPE(value), self->value_type))
    {
        PyErr_Format(PyExc_TypeError, "Field `%s` of `%s` must be`%s` or its subclasses, can't assign instance of `%s`", self->name, self->instance_type->tp_name, self->value_type->tp_name, Py_TYPE(value)->tp_name);
        return -1;
    }

    Py_XDECREF(field);
    Py_INCREF(value);
    field = value;
    return 0;
}

int Property_Set(Property_Object const *self, PyObject *instance, PyObject *value)
{
    if (!PyType_IsSubtype(Py_TYPE(instance), self->instance_type))
    {
        PyErr_Format(PyExc_TypeError, "Can't set member `%s` to `%s`, must be `%s` or its subclasses", self->name, Py_TYPE(instance)->tp_name, self->instance_type->tp_name);
        return -1;
    }

    if (!self->mutable)
    {
        PyErr_Format(PyExc_TypeError, "Field `%s` of `%s` is immutable and can't be set or deleted", self->name, self->instance_type->tp_name);
        return -1;
    }

    return Property_SetNoCheck(self, instance, value);
}

#undef field

static PyObject *Property_GetOffset(Property_Object *self, void *closure)
{
    return PyLong_FromSize_t(self->offset);
}

static PyObject *Property_GetIsOptional(Property_Object *self, void *closure)
{
    retPyBool(self->optional);
}


static PyObject *Property_GetIsMutable(Property_Object *self, void *closure)
{
    retPyBool(self->mutable);
}

static void Property_Dealloc(Property_Object *self)
{}


PyObject *Parse_ArgsKwargsToDict(
        PyObject *args,
        PyObject *kwargs
)
{
    ifNull(kwargs)
    {
        if (!PyArg_ParseTuple(args, "O!;Pass json object or specify kwargs with data", &PyDict_Type, &kwargs))
        {
            return NULL;
        }
    }
    else
    {
        if (!PyArg_ParseTuple(args, ";Pass json object or specify kwargs with data"))
        {
            return NULL;
        }
    }

    if (!PyArg_ValidateKeywordArguments(kwargs))
    {
        return NULL;
    }

    return kwargs;
}

PyObject *Parse_DictToProperties(
        PyObject *data,
        char const *const *ignore_keywords,
        PyTypeObject *cls,
        Py_ssize_t ob_length,
        size_t meta_quantity,
        ...
)
{
    PyObject *self = NULL;
    va_list varargs;
    PyObject *existing_keys;
    PyObject *key;
    Property_Object const *meta;
    size_t meta_left;


    ifNullRet(existing_keys = PySet_New(data));

    for (; *ignore_keywords != NULL; ignore_keywords++)
    {
        ifNull(key = PyUnicode_FromString(*ignore_keywords))
        {
            Py_DECREF(existing_keys);
            return NULL;
        }
        if (PySet_Discard(existing_keys, key) == -1)
        {
            Py_DECREF(key);
            Py_DECREF(existing_keys);
            return NULL;
        }
        Py_DECREF(key);
    }

    va_start(varargs, meta_quantity);
    for (meta_left = meta_quantity; meta_left > 0; meta_left--)
    {
        for (meta = va_arg(varargs, Property_Object *); meta->name != NULL; meta++)
        {
            ifNull(key = PyUnicode_FromString(meta->name))
            {
                Py_DECREF(existing_keys);
                va_end(varargs);
                return NULL;
            }
            if (PySet_Discard(existing_keys, key) == -1)
            {
                Py_DECREF(key);
                Py_DECREF(existing_keys);
                va_end(varargs);
                return NULL;
            }
            Py_DECREF(key);
        }
    }
    va_end(varargs);

    if (PySet_GET_SIZE(existing_keys) > 0)
    {
        PyErr_Format(PyExc_AttributeError, "Unexpected arguments for constructor of class `%s` %S", cls->tp_name, existing_keys);
        Py_DECREF(existing_keys);
        return NULL;
    }
    Py_DECREF(existing_keys);

    ifNull(self = cls->tp_alloc(cls, ob_length))
    {
        PyErr_NoMemory();
        return NULL;
    }

    va_start(varargs, meta_quantity);
    for (meta_left = meta_quantity; meta_left > 0; meta_left--)
    {
        for (meta = va_arg(varargs, Property_Object *);meta->name != NULL;meta++)
        {
            if (Property_SetNoCheck(meta, self, PyDict_GetItemString(data, meta->name)))
            {
                Py_DECREF(self);
                va_end(varargs);
                return NULL;
            }
        }
    }
    va_end(varargs);

    return self;
}

int Property_StoreToClass(PyTypeObject *cls, Property_Object *array)
{
    ifNull(cls->tp_dict)
    {
        ifNull(cls->tp_dict = PyDict_New())
        {
            return 1;
        }
    }

    for (; array->name != NULL; array++)
    {
        if (PyDict_SetItemString(cls->tp_dict, array->name, (PyObject *) array))
        {
            return 1;
        }
    }
    return 0;
}

FileInitFunction(Property)
{

    chkType(Property_Type);
    modAddO("_property", &Property_Type);

    return 0;
}