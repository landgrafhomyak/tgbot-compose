#ifndef API_TYPES_MACRO_H
# define API_TYPES_MACRO_H

# define ifne(EXPRESSION) if ((EXPRESSION) < 0)
# define parsePyBool(PY_OBJECT) ((PY_OBJECT == Py_True) ? 1 : 0)
# define makePyBool(BOOL) ((BOOL) ? Py_True : Py_False)
# define retPyBool(BOOL) if (BOOL) Py_RETURN_TRUE; else Py_RETURN_FALSE; ((void)0)
# define ifNull(EXPRESSION) if ((EXPRESSION) == NULL)
# define ifNullRet(EXPRESSION) if ((EXPRESSION) == NULL) return NULL; ((void)0)

#endif /* API_TYPES_MACRO_H */
