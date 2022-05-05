project(api_types C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_executable(
        api_types__generator
        ./generator.cpp
)


foreach (INC_FILE_NAME_PREFIX ./User)
    add_custom_command(
            OUTPUT ${INC_FILE_NAME_PREFIX}.inc
            COMMAND $<TARGET_FILE:api_types__generator> ${INC_FILE_NAME_PREFIX}.txt ${INC_FILE_NAME_PREFIX}.inc
            DEPENDS api_types__generator ${INC_FILE_NAME_PREFIX}.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach ()

py_module(
        api_types

        ./macro.h
        # ./macro.hpp
        ./__init__.c
        ./__init__.h
        # ./dict.cpp
        # ./dict.hpp
        serialization.h
        ./property.c
        # ./property.hpp
        ./property.h
        ./bases.c
        ./bases.h
        ./User.c
        ./User.inc
        # ./User.cpp
)

add_dependencies(api_types api_types__generator)
