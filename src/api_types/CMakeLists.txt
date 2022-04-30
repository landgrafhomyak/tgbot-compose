project(api_types C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

py_module(
        api_types

        ./macro.h
        ./macro.hpp
        ./__init__.c
        ./__init__.h
        ./dict.cpp
        ./dict.hpp
        ./property.c
        # ./property.hpp
        ./property.h
        ./bases.c
        ./bases.h
        # ./User.c
        ./User.cpp
)

install(
        TARGETS api_types DESTINATION ./
)