#ifndef API_TYPES_BASES_H
# define API_TYPES_BASES_H

# ifdef __cplusplus
extern "C" {
# endif

# include <Python.h>

extern PyTypeObject ApiTypeBase_Type;

# define apiType(DERIVED_TYPE_SCALAR) ((DERIVED_TYPE_SCALAR).tp_base = &ApiTypeBase_Type)

# ifdef __cplusplus
};
# endif

#endif /* API_TYPES_BASES_H */
