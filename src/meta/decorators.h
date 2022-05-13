#include <Python.h>

#ifndef META_DECORATORS_H
# define META_DECORATORS_H

# ifdef __cplusplus
extern "C" {
# endif

typedef struct ExportDecorator_Object
{
    PyObject_HEAD
    PyObject *member;
} ExportDecorator_Object;

extern PyTypeObject ExportDecorator_Type;

#define ExportDecorator_Check(OBJECT) (Py_TYPE((OBJECT)) == &ExportDecorator_Type)

#define ExportDecorator_Get(OBJECT) (((ExportDecorator_Object *)(OBJECT))->member)

# ifdef __cplusplus
};
# endif

#endif /* META_DECORATORS_H */
