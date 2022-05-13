#include <Python.h>
#include "__init__.h"
#include "decorators.h"


PyTypeObject ExportDecorator_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)

};


FileInitFunction(Decorators)
{
    return 0;
}