#ifndef API_TYPES___INIT___H
# define API_TYPES___INIT___H

# ifdef __cplusplus
extern "C" {
# endif

# include <Python.h>
# include "macro.h"

# define chkType(TYPE_SCALAR) if(PyType_Ready(&(TYPE_SCALAR)) != 0) { return 1; } ((void)0)
# define modAddO(NAME, OBJECT) ifne(PyModule_AddObject(module, (NAME), (PyObject *)(OBJECT))) { return 1; } ((void)0)

# define FileInitFunction(NAME) int (Init_ ## NAME)(PyObject *module)

FileInitFunction(Property);

FileInitFunction(Bases);

FileInitFunction(User);

# ifdef __cplusplus
};
# endif

#endif /* API_TYPES___INIT___H */
