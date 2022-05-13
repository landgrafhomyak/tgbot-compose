#include <Python.h>

#ifndef META_UNIT_H
# define META_UNIT_H

# ifdef __cplusplus
extern "C" {
# endif

typedef struct UnitMeta_Object
{
    PyObject_HEAD
    PyObject * /* set[str] */ exports;
    PyObject * /* tuple[unit_meta, ...] */ dependencies;
} UnitMeta_Object;

extern PyTypeObject UnitMeta_Type;

# define UnitMeta_Check(OBJECT) (Py_TYPE((OBJECT)) == &UnitMeta_Type)

# ifdef __cplusplus
};
# endif

#endif /* META_UNIT_H */
