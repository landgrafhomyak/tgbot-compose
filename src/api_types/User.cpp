#include <cstddef>

#include <initializer_list>

#include <Python.h>

#include "macro.h"
#include "__init__.h"
#include "property.h"
#include "macro.hpp"
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

static User_Object *User_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void User_Dealloc(User_Object *self);

static PyObject *User_GetIsBot(User_Object *self, void *closure);

static PyObject *User_GetFullName(User_Object *self, void *closure);

static PyGetSetDef User_GetSet[] = {
        {"is_bot",    (getter) User_GetIsBot,    NULL},
        {"full_name", (getter) User_GetFullName, NULL},
        {NULL}
};

static PyTypeObject User_Type = {
        CxxPyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.User",
        .tp_basicsize = sizeof(User_Object),
        .tp_dealloc = (destructor) User_Dealloc,
        .tp_getset = User_GetSet,
        .tp_new = (newfunc) User_New,
};

static RealUser_Object *RealUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void RealUser_Dealloc(RealUser_Object *self);

static PyObject *RealUser_Repr(RealUser_Object *self);

static PyTypeObject RealUser_Type = {
        CxxPyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.RealUser",
        .tp_basicsize = sizeof(RealUser_Object),
        .tp_dealloc = (destructor) RealUser_Dealloc,
        .tp_repr = (reprfunc) RealUser_Repr,
        .tp_base = (PyTypeObject *const) &User_Type,
        .tp_new = (newfunc) RealUser_New,
};


static BotUser_Object *BotUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void BotUser_Dealloc(BotUser_Object *self);

static PyObject *BotUser_Repr(BotUser_Object *self);


static PyTypeObject BotUser_Type = {
        CxxPyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotUser",
        .tp_basicsize = sizeof(BotUser_Object),
        .tp_dealloc = (destructor) BotUser_Dealloc,
        .tp_repr = (reprfunc) &BotUser_Repr,
        .tp_base = (PyTypeObject *const) &User_Type,
        .tp_new = (newfunc) BotUser_New,
};

static BotUser_Object *BotSelfUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void BotSelfUser_Dealloc(BotSelfUser_Object *self);

static PyObject *BotSelfUser_Repr(BotSelfUser_Object *self);

static PyTypeObject BotSelfUser_Type = {
        CxxPyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotSelfUser",
        .tp_basicsize = sizeof(BotSelfUser_Object),
        .tp_dealloc = (destructor) BotSelfUser_Dealloc,
        .tp_repr = (reprfunc) BotSelfUser_Repr,
        .tp_base = (PyTypeObject *const) &BotUser_Type,
        .tp_new = (newfunc) BotSelfUser_New,
};

static PyObject *TrueGetter()
{
    Py_RETURN_TRUE;
}

static PyObject *FalseGetter()
{
    Py_RETURN_FALSE;
}

static constexpr DictMeta User_Meta(
        User_Type,
        nullptr,
        prop(id, User_Object, PyLong_Type, nullptr),
        prop(first_name, User_Object, PyUnicode_Type, nullptr),
        prop(username, User_Object, PyUnicode_Type, DictMeta_GetNone)
);

static constexpr DictMeta RealUser_Meta(
        RealUser_Type,
        &User_Meta,
        DictMeta::DictMeta_InputEntry{.name = "is_bot", .offset=(size_t) -1, .required_type=PyBool_Type, .default_value_getter=FalseGetter},
        prop(last_name, RealUser_Object, PyUnicode_Type, DictMeta_GetNone),
        prop(language_code, RealUser_Object, PyUnicode_Type, DictMeta_GetNone)
);

static constexpr DictMeta BotUser_Meta(
        BotUser_Type,
        &User_Meta,
        DictMeta::DictMeta_InputEntry{.name = "is_bot", .offset=(size_t) -1, .required_type=PyBool_Type, .default_value_getter=TrueGetter},
        prop(username, BotUser_Object, PyUnicode_Type, nullptr)
);
static constexpr DictMeta BotSelfUser_Meta(
        BotSelfUser_Type,
        &BotUser_Meta,
        prop(can_join_groups, BotSelfUser_Object, PyBool_Type, nullptr),
        prop(can_read_all_group_messages, BotSelfUser_Object, PyBool_Type, nullptr),
        prop(supports_inline_queries, BotSelfUser_Object, PyBool_Type, nullptr)
);


static char const *const ignore_names[] = {
        "is_bot",
        NULL
};

static inline PyObject *check_is_bot(PyTypeObject *cls, PyObject *data, PyObject *expected)
{
    PyObject *is_bot;

    is_bot = PyDict_GetItemString(data, "is_bot");
    if (is_bot == NULL)
    {
        PyErr_Format(PyExc_AttributeError, "Argument `is_bot` wasn't passed to constructor of `%s`", User_Type.tp_name);
        return NULL;
    }

    if (!PyBool_Check(is_bot))
    {
        PyErr_Format(PyExc_TypeError, "Argument `is_bot` of constructor of class '%s' must be `bool`, not `%s`", User_Type.tp_name, Py_TYPE(is_bot)->tp_name);
        return NULL;
    }

    if (expected == NULL)
    {
        return is_bot;
    }

    if (is_bot != expected)
    {
        PyErr_Format(PyExc_ValueError, "Argument `is_bot` of constructor of class `%s` must be %S", User_Type.tp_name, is_bot);
        return NULL;
    }
    return is_bot;
}

static RealUser_Object *RealUser_Create(PyObject *data);

static BotUser_Object *BotUser_Create(PyObject *data);

static User_Object *User_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    PyObject *is_bot;

    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));

    ifNullRet(is_bot = check_is_bot(&User_Type, kwargs, NULL));

    if (parsePyBool(is_bot))
    {
        return (User_Object *) BotUser_Create(kwargs);
    }
    else
    {
        return (User_Object *) RealUser_Create(kwargs);
    }
}

static void User_Dealloc(User_Object *self)
{
    PyErr_Format(PyExc_RuntimeError, "Destructor of class `%s` called for %R", User_Type.tp_name, Py_TYPE(self));
}

static PyObject *User_GetIsBot(User_Object *self, void *closure)
{
    if (Py_TYPE(self) == &User_Type)
    {
        PyErr_Format(PyExc_TypeError, "Interface `%s` can't be bot or not", User_Type.tp_name);
        return NULL;
    }

    if (Py_TYPE(self) == &RealUser_Type)
    {
        Py_RETURN_FALSE;
    }

    if (Py_TYPE(self) == &BotUser_Type || Py_TYPE(self) == &BotSelfUser_Type)
    {
        Py_RETURN_FALSE;
    }

    PyErr_Format(PyExc_TypeError, "Can check for bots only instances of subclasses of `%s`", User_Type.tp_name);
    return NULL;
}

static PyObject *User_GetFullName(User_Object *self, void *closure)
{
    if (Py_TYPE(self) != &RealUser_Type)
    {
        if (Py_TYPE(self) != &BotUser_Type && Py_TYPE(self) != &BotSelfUser_Type)
        {
            PyErr_Format(PyExc_TypeError, "Full name can be obtained from instances of subclasses of `%s`", User_Type.tp_name);
            return NULL;
        }
        return Py_NewRef(self->first_name);
    }
    else
    {
        ifNull(((RealUser_Object *) self)->last_name)
        {
            return Py_NewRef(self->first_name);
        }
        else
        {
            return PyUnicode_FromFormat("%s %s", self->first_name, ((RealUser_Object *) self)->last_name);
        }
    }
}

static RealUser_Object *RealUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));

    ifNullRet(check_is_bot(&RealUser_Type, kwargs, Py_False));

    return RealUser_Create(kwargs);
}


static RealUser_Object *RealUser_Create(PyObject *data)
{
    return (RealUser_Object *) Parse_DictToProperties(data, ignore_names, &RealUser_Type, 0, 2, User_Meta, RealUser_Meta);
}

static void RealUser_Dealloc(RealUser_Object *self)
{
    Py_XDECREF(self->id);
    Py_XDECREF(self->first_name);
    Py_XDECREF(self->username);
    Py_XDECREF(self->last_name);
    Py_XDECREF(self->language_code);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *RealUser_Repr(RealUser_Object *self)
{
    PyObject *repr;
    PyObject *full_name;

    ifNullRet(full_name = User_GetFullName((User_Object *) self, NULL));

    repr = PyUnicode_FromFormat(
            "<%s object id=%S username=%V full_name=%R language_code=%R>",
            RealUser_Type.tp_name,
            self->id,
            self->username,
            "@",
            full_name,
            self->language_code
    );

    Py_DECREF(full_name);
    return repr;
}


static BotUser_Object *BotUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));

    ifNullRet(check_is_bot(&BotUser_Type, kwargs, Py_False));

    return BotUser_Create(kwargs);
}

static BotUser_Object *BotUser_Create(PyObject *data)
{
    return (BotUser_Object *) Parse_DictToProperties(data, ignore_names, &BotUser_Type, 0, 2, User_Meta, BotUser_Meta);
}


static void BotUser_Dealloc(BotUser_Object *self)
{
    Py_XDECREF(self->id);
    Py_XDECREF(self->first_name);
    Py_XDECREF(self->username);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *BotUser_Repr(BotUser_Object *self)
{
    return PyUnicode_FromFormat(
            "<%s object id=%S username=%U name=%R>",
            BotUser_Type.tp_name,
            self->id,
            self->username,
            self->first_name
    );
}

static BotUser_Object *BotSelfUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));

    ifNullRet(check_is_bot(&BotSelfUser_Type, kwargs, Py_False));

    return (BotUser_Object *) Parse_DictToProperties(kwargs, ignore_names, &BotSelfUser_Type, 0, 3, User_Meta, BotUser_Meta, BotSelfUser_Meta);
}

static void BotSelfUser_Dealloc(BotSelfUser_Object *self)
{
    Py_XDECREF(self->id);
    Py_XDECREF(self->first_name);
    Py_XDECREF(self->username);
    Py_XDECREF(self->supports_inline_queries);
    Py_XDECREF(self->can_join_groups);
    Py_XDECREF(self->can_read_all_group_messages);
    Py_TYPE(self)->tp_free(self);
}


static PyObject *BotSelfUser_Repr(BotSelfUser_Object *self)
{
    return PyUnicode_FromFormat(
            "<%s object id=%S username=%U name=%R%s%s%s>",
            RealUser_Type.tp_name,
            self->id,
            self->username,
            self->first_name,
            (parsePyBool(self->can_join_groups) ? " in groups" : ""),
            (parsePyBool(self->can_read_all_group_messages) ? " can read all" : ""),
            (parsePyBool(self->supports_inline_queries) ? " with inline" : "")
    );
}

FileInitFunction(User)
{
    return 0;
}