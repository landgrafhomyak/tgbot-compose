cmake_minimum_required(VERSION 3.0)
project(telegrambotcompose C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

if (SKBUILD)
    find_package(PythonExtensions REQUIRED)
    include(UsePythonExtensions)

    function(py_module LIB_NAME)
        add_python_extension(
                ${LIB_NAME}
                SOURCES ${ARGN}
        )
    endfunction()

else ()
    find_package(Python COMPONENTS Development REQUIRED)

    function(py_module LIB_NAME)
        # message(ERROR compile possible only with scikit-build)
        python_add_library(
                ${LIB_NAME}
                MODULE WITH_SOABI
                ${ARGN}
        )
    endfunction()
endif ()

add_subdirectory(./src/api_types)