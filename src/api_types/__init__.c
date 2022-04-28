#include <Python.h>
#include "__init__.h"


#define init_file(NAME) if(Init_ ## NAME(module) != 0) { Py_DECREF(module); return NULL; } ((void)0)

PyMODINIT_FUNC PyInit_api_types(void)
{
    PyObject *module;
    static PyModuleDef module_def = {
            PyModuleDef_HEAD_INIT,
            .m_name = "tgbot_compose.api_types",
            .m_size = -1
    };

    ifNullRet(module = PyModule_Create(&module_def));

    init_file(Property);
    init_file(Bases);
    init_file(User);

    return module;
}