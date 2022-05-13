project(api_types C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

py_module(
        meta

        ./__init__.c
        ./__init__.h
        ./macro.h
        ./unit.c
        ./unit.h
        ./decorators.c
        ./decorators.h
)