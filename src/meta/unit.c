#include <Python.h>
#include "__init__.h"
#include "decorators.h"

typedef struct UnitMeta_Object
{
    PyObject_HEAD
    PyObject *export_dict;
} UnitMeta_Object;

static PyObject *UnitMetaFactory_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs);

static PyTypeObject UnitMetaFactory_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.meta.UnitMetaFactory",
        .tp_new = UnitMetaFactory_New
};

static int sequence_converter(PyObject *source, PyObject **destination)
{
    if (!PySequence_Check(source))
    {
        PyErr_Format(PyExc_TypeError, "Dependencies list must support sequence protocol");
        return 0;
    }
    *destination = source;
    return 1;
}

static int mapping_converter(PyObject *source, PyObject **destination)
{
    if (!PyMapping_Check(source))
    {
        PyErr_Format(PyExc_TypeError, "Namespace dict must support mapping protocol");
        return 0;
    }
    *destination = source;
    return 1;
}


static PyObject *UnitMetaFactory_New(PyTypeObject *mcs, PyObject *args, PyObject *kwargs)
{
    static char const *const kw_names[] = {"name", "dependencies", "namespace", NULL};
    PyObject *name;
    PyObject *dependencies;
    PyObject *namespace;
//    struct
//    {
//        PyObject *tuple;
//        PyObject *set;
//        PyObject *fields;
//        Py_ssize_t iterator_pos;
//        PyObject *name;
//        PyObject *type;
//        PyObject *intersection;
//    } slots;
    struct
    {
        PyObject *local_exports;
        Py_ssize_t iterator_pos;
        PyObject *name;
        PyObject *member;
        PyObject *inherited_exports;
    } export;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!", (char **) kw_names, &PyUnicode_Type, &name, &PyTuple_Type, &dependencies, &PyDict_Type, &namespace))
    { return NULL; }

    /*
     * @refs
     * Getting __annotations__
     */

    PyObject *fields = PyDict_GetItemString(namespace, "__annotations__");
    if (fields == NULL)
    {
        fields = PyDict_New();
        if (fields == NULL)
        {
            return NULL;
        }
    }
    else if (!PyDict_Check(fields))
    {
        PyErr_Format(PyExc_TypeError, "__annotations__ must be dict");
        return NULL;
    }

    /*
     * @refs fields
     * Getting __slots__ and merging to fields
     */
    {
        PyObject *slots = PyDict_GetItemString(namespace, "__slots__");
        if (slots != NULL)
        {
            PyObject *seq = PySequence_Fast(slots, "__slots__ is not iterable");
            Py_DECREF(slots);
            if (seq == NULL)
            {
                Py_DECREF(fields);
                return NULL;
            }
            PyObject **seqp = PySequence_Fast_ITEMS(seq);

            for (Py_ssize_t pos = PySequence_Fast_GET_SIZE(seq) - 1; pos >= 0; pos--, seqp++)
            {

            }
        }
    }

    PyObject *slots;
    {
        slots.tuple = PyDict_GetItemString(namespace, "__slots__");
        if (slots.tuple == NULL)
        { slots.set = PySet_New(NULL); }
        else
        {
            slots.set = PySet_New(slots.tuple);
            if (slots.set == NULL)
            { return NULL; }
        }
    }
    /*
     * @refs slots.set
     * Getting __annotations__ and excluding its keys from __slots__
     */

    slots.annotations = PyDict_GetItemString(namespace, "__annotations__");
    if (slots.annotations != NULL)
    {
        if (!PyDict_Check(slots.annotations))
        {
            PyErr_Format(PyExc_TypeError, "__annotations__ must be a dict");
            return NULL;
        }
        slots.annotations = PyDict_Copy(slots.annotations);
        if (slots.annotations == NULL)
        {
            Py_DECREF(slots.set);
            return NULL;
        }
        slots.iterator_pos = 0;
        while (PyDict_Next(slots.annotations, &(slots.iterator_pos), &(slots.name), &(slots.type)))
        {
            switch (PySet_Discard(slots.set, slots.name))
            {
                case 1:
                    switch (PyErr_WarnFormat(PyExc_UserWarning, 1, "Duplication of property %R in __slots__ and __annotations__", slots.name))
                    {
                        case 0:
                            break;
                        case -1:
                            Py_DECREF(slots.set);
                            Py_DECREF(slots.annotations);
                            return NULL;
                        default:
                            Py_DECREF(slots.set);
                            Py_DECREF(slots.annotations);
                            Py_FatalError("'PyErr_WarnFormat' returned unexpected value (not 0 or -1) in python version " PY_VERSION);
                            return NULL;
                    }
                case 0:
                    break;
                case -1:
                    Py_DECREF(slots.set);
                    Py_DECREF(slots.annotations);
                    return NULL;
                default:
                    Py_DECREF(slots.set);
                    Py_DECREF(slots.annotations);
                    Py_FatalError("'PySet_Discard' returned unexpected value (not 1, 0 or -1) in python version " PY_VERSION);
                    return NULL;
            }
        }

    }
    else
    {
        slots.annotations = PyDict_New();
        if (slots.annotations == NULL)
        {
            Py_DECREF(slots.set);
            return NULL;
        }
    }

    /*
     * @refs slots.set slots.fields
     * Saving __slots__
     */

    slots.tuple = PySequence_Tuple(slots.set);
    Py_DECREF(slots.set);
    if (slots.tuple == NULL)
    {
        Py_DECREF(slots.annotations);
        return NULL;
    }
    switch (PyDict_SetItemString(namespace, "__slots__", slots.tuple))
    {
        case 0:
            break;
        case -1:
            Py_DECREF(slots.tuple);
        default:
            Py_DECREF(slots.tuple);
            Py_DECREF(slots.annotations);
            Py_FatalError("'PyErr_WarnFormat' returned unexpected value (not 0 or -1) in python version " PY_VERSION);
            return NULL;
    }
    Py_DECREF(slots.tuple);

    /*
     * @refs slots.fields
     * Checking for intersection between __annotations__ and namespace
     */

    slots.intersection = PyNumber_And(namespace, slots.annotations);
    if (slots.intersection == NULL)
    {
        Py_DECREF(slots.annotations);
        return NULL;
    }
    switch (PyObject_Length(slots.intersection))
    {
        case -1:
            Py_DECREF(slots.intersection);
            Py_DECREF(slots.annotations);
            return NULL;
        case 0:
            break;
        default:
            Py_DECREF(slots.annotations);
            PyErr_Format(PyExc_ValueError, "Some names duplicates in __annotations__ and namespace: %R", slots.intersection);
            Py_DECREF(slots.intersection);
            return NULL;
    }
    Py_DECREF(slots.intersection);

    /*
     * @refs slots.fields
     * Collecting local symbols for export
     */

    export.local_exports = PySet_New(NULL);
    if (export.local_exports == NULL)
    {
        Py_DECREF(slots.annotations);
        return NULL;
    }
    export.iterator_pos = 0;
    while (PyDict_Next(namespace, &(export.iterator_pos), &(export.name), &(export.member)))
    {
        if (ExportDecorator_Check(export.member))
        {
            switch (PyDict_SetItem(namespace, export.name, ExportDecorator_Get(export.member)))
            {
                case 0:
                    break;
                case -1:
                    Py_DECREF(export.local_exports);
                    Py_DECREF(slots.annotations);
                    return NULL;
                default:
                    Py_DECREF(export.local_exports);
                    Py_DECREF(slots.annotations);
                    Py_FatalError("'PyDict_SetItem' returned unexpected value (not 0 or -1) in python version " PY_VERSION);
                    return NULL;
            }
        }
        switch (PySet_Add(export.local_exports, export.name))
        {
            case 0:
                break;
            case -1:
                Py_DECREF(export.local_exports);
                Py_DECREF(slots.annotations);
                return NULL;
            default:
                Py_DECREF(export.local_exports);
                Py_DECREF(slots.annotations);
                Py_FatalError("'PySet_Add' returned unexpected value (not 0 or -1) in python version " PY_VERSION);
                return NULL;
        }
    }

    /*
     * @refs slots.fields export.local_exports
     * Collecting inherited symbols for export
     */

    export.inherited_exports = PySet_New(NULL);
    for (export.iterator_pos = 0; export.iterator_pos < PyTuple_GET_SIZE(dependencies); export.iterator_pos++)
    {
        UnitMeta_Object *dependency = (UnitMeta_Object *) PyTuple_GET_ITEM(dependencies, export.iterator_pos);
        PyObject *u = PyNumber_InPlaceOr(export.inherited_exports, dependency->export_dict);
        if (u == NULL)
        {
            Py_DECREF(slots.annotations);
            Py_DECREF(export.local_exports);
            return NULL;
        }
        Py_DECREF(u);
    }


}

FileInitFunction(Unit)
{
    chkType(UnitMetaFactory_Type);
    return 0;
}