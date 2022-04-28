#ifndef API_TYPES_BASES_H
# define API_TYPES_BASES_H

# include <Python.h>

extern PyTypeObject ApiTypeBase_Type;

# define apiType(DERIVED_TYPE_SCALAR) ((DERIVED_TYPE_SCALAR).tp_base = &ApiTypeBase_Type)

#endif /* API_TYPES_BASES_H */
