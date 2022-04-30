#include <cstddef>

#include <initializer_list>

#include <Python.h>

#include "dict.hpp"

struct User_Object
{
    PyObject_HEAD
    PyObject *const id;
    PyObject *const first_name;
    PyObject *const username;
};

struct RealUser_Object : public User_Object
{
    PyObject *const last_name;
    PyObject *const language_code;
};

struct BotUser_Object : public User_Object
{
};

struct BotSelfUser_Object : public BotUser_Object
{
    PyObject *const can_join_groups;
    PyObject *const can_read_all_group_messages;
    PyObject *const supports_inline_queries;
};

static PyTypeObject x = {

};

static constexpr DictMeta User_Meta(
        x,
        nullptr,
        prop(id, User_Object, PyLong_Type, nullptr),
        prop(first_name, User_Object, PyUnicode_Type, nullptr),
        prop(username, User_Object, PyUnicode_Type, DictMeta_GetNone)
);

static constexpr DictMeta RealUser_Meta(
        x,
        &User_Meta,
        prop(last_name, RealUser_Object, PyUnicode_Type, DictMeta_GetNone),
        prop(language_code, RealUser_Object, PyUnicode_Type, DictMeta_GetNone)
);

static constexpr DictMeta BotUser_Meta(
        x,
        &User_Meta,
        prop(username, BotUser_Object, PyUnicode_Type, nullptr)
);
static constexpr DictMeta BotSelfUser_Meta(
        x,
        &BotUser_Meta,
        prop(can_join_groups, BotSelfUser_Object, PyBool_Type, nullptr),
        prop(can_read_all_group_messages, BotSelfUser_Object, PyBool_Type, nullptr),
        prop(supports_inline_queries, BotSelfUser_Object, PyBool_Type, nullptr)
);
