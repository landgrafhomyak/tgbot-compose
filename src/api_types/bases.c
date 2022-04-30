#include <Python.h>

#include "__init__.h"
#include "bases.h"



PyTypeObject ApiTypeBase_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types._ApiTypeBase",
};

FileInitFunction(Bases)
{
    chkType(ApiTypeBase_Type);

    return 0;
}