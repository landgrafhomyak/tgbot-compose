#include <stddef.h>

#include <Python.h>

#include "macro.h"
#include "__init__.h"
#include "bases.h"
#include "property.h"

typedef struct User_Object
{
    PyObject_HEAD
    PyObject *const id;
    PyObject *const first_name;
    PyObject *const username;
} User_Object;

typedef struct RealUser_Object
{
    User_Object common;
    PyObject *const last_name;
    PyObject *const language_code;
} RealUser_Object;

typedef struct BotUser_Object
{
    User_Object common;
} BotUser_Object;

typedef struct BotSelfUser_Object
{
    BotUser_Object bot;
    PyObject *const can_join_groups;
    PyObject *const can_read_all_group_messages;
    PyObject *const supports_inline_queries;
} BotSelfUser_Object;

static PyTypeObject User_Type;
static PyTypeObject RealUser_Type;
static PyTypeObject BotUser_Type;
static PyTypeObject BotSelfUser_Type;

#include "User.inc"

static User_Object *User_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void User_Dealloc(User_Object *self);

static PyObject *User_GetFullName(User_Object *self, void *closure);

static PyGetSetDef User_GetSet[] = {
        User_GetSet_GENERATED
        {"full_name", (getter) User_GetFullName, NULL},
        {NULL}
};

static PyTypeObject User_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.User",
        .tp_basicsize = sizeof(User_Object),
        .tp_new = (newfunc) User_New,
        .tp_getset = User_GetSet,
        .tp_dealloc = (destructor) User_Dealloc,
};

static PyObject *RealUser_Repr(RealUser_Object *self);

static PyObject *RealUser_GetFullName(User_Object *self, void *closure);

static PyGetSetDef RealUser_GetSet[] = {
        RealUser_GetSet_GENERATED
        {"full_name", (getter) RealUser_GetFullName, NULL},
        {NULL}
};

static PyTypeObject RealUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.RealUser",
        .tp_basicsize = sizeof(RealUser_Object),
        .tp_new = (newfunc) RealUser_New,
        .tp_dealloc = (destructor) RealUser_Dealloc,
        .tp_base = &User_Type,
        .tp_repr = (reprfunc) RealUser_Repr,
        .tp_getset = RealUser_GetSet
};

static PyObject *BotUser_Repr(BotUser_Object *self);

static PyObject *BotUser_GetFullName(User_Object *self, void *closure);

static PyGetSetDef BotUser_GetSet[] = {
        BotUser_GetSet_GENERATED
        {"full_name", (getter) BotUser_GetFullName, NULL},
        {NULL}
};

static PyTypeObject BotUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotUser",
        .tp_basicsize = sizeof(BotUser_Object),
        .tp_new = (newfunc) BotUser_New,
        .tp_dealloc = (destructor) BotUser_Dealloc,
        .tp_base = &User_Type,
        .tp_repr = (reprfunc) &BotUser_Repr,
        .tp_getset = BotUser_GetSet
};

static PyObject *BotSelfUser_Repr(BotSelfUser_Object *self);

static PyGetSetDef BotSelfUser_GetSet[] = {
        BotSelfUser_GetSet_GENERATED
        {NULL}
};
static PyTypeObject BotSelfUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotSelfUser",
        .tp_basicsize = sizeof(BotSelfUser_Object),
        .tp_new = (newfunc) BotSelfUser_New,
        .tp_dealloc = (destructor) BotSelfUser_Dealloc,
        .tp_base = &BotUser_Type,
        .tp_repr = (reprfunc) BotSelfUser_Repr,
        .tp_getset = BotSelfUser_GetSet
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

static User_Object *User_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    PyObject *is_bot;

//    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));
//
//    ifNullRet(is_bot = check_is_bot(&User_Type, kwargs, NULL));
//
//    if (parsePyBool(is_bot))
//    {
//        return (User_Object *) BotUser_Create(kwargs);
//    }
//    else
//    {
//        return (User_Object *) RealUser_Create(kwargs);
//    }
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
        return RealUser_GetFullName(self, closure);
    }
}

static PyObject *RealUser_Repr(RealUser_Object *self)
{
    PyObject *repr;
    PyObject *full_name;

    ifNullRet(full_name = User_GetFullName((User_Object *) self, NULL));

    repr = PyUnicode_FromFormat(
            "<%s object id=%S username=%V full_name=%R language_code=%R>",
            RealUser_Type.tp_name,
            self->common.id,
            self->common.username,
            "@",
            full_name,
            self->language_code
    );

    Py_DECREF(full_name);
    return repr;
}

static PyObject *RealUser_GetFullName(User_Object *self, void *closure)
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

static PyObject *BotUser_Repr(BotUser_Object *self)
{
    return PyUnicode_FromFormat(
            "<%s object id=%S username=%U name=%R>",
            BotUser_Type.tp_name,
            self->common.id,
            self->common.username,
            self->common.first_name
    );
}

static PyObject *BotUser_GetFullName(User_Object *self, void *closure)
{
    return Py_NewRef(self->first_name);
}

static PyObject *BotSelfUser_Repr(BotSelfUser_Object *self)
{
    return PyUnicode_FromFormat(
            "<%s object id=%S username=%U name=%R%s%s%s>",
            RealUser_Type.tp_name,
            self->bot.common.id,
            self->bot.common.username,
            self->bot.common.first_name,
            (parsePyBool(self->can_join_groups) ? " in groups" : ""),
            (parsePyBool(self->can_read_all_group_messages) ? " can read all" : ""),
            (parsePyBool(self->supports_inline_queries) ? " with inline" : "")
    );
}

FileInitFunction(User)
{
    apiType(User_Type);

    chkType(User_Type);
    chkType(RealUser_Type);
    chkType(BotUser_Type);
    chkType(BotSelfUser_Type);

    modAddO("User", &User_Type);
    modAddO("RealUser", &RealUser_Type);
    modAddO("BotUser", &BotUser_Type);
    modAddO("BotSelfUser", &BotSelfUser_Type);

    return 0;
}