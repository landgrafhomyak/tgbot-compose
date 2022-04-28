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
    BotUser_Object common;
    PyObject *const can_join_groups;
    PyObject *const can_read_all_group_messages;
    PyObject *const supports_inline_queries;
} BotSelfUser_Object;

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
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.User",
        .tp_basicsize = sizeof(User_Object),
        .tp_new = (newfunc) User_New,
        .tp_getset = User_GetSet,
        .tp_dealloc = (destructor) User_Dealloc,
};


static Property_Object User_Meta[] = {
        Property_SInit("id", 0, 0, offsetof(User_Object, id), NULL),
        Property_SInit("first_name", 0, 0, offsetof(User_Object, first_name), NULL),
        Property_SInit("username", 0, 1, offsetof(User_Object, username), NULL),
        Property_ListEnd
};


static RealUser_Object *RealUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void RealUser_Dealloc(RealUser_Object *self);

static PyObject *RealUser_Repr(RealUser_Object *self);

static PyTypeObject RealUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.RealUser",
        .tp_basicsize = sizeof(RealUser_Object),
        .tp_new = (newfunc) RealUser_New,
        .tp_dealloc = (destructor) RealUser_Dealloc,
        .tp_base = &User_Type,
        .tp_repr = (reprfunc) RealUser_Repr
};

static Property_Object RealUser_Meta[] = {
        Property_SInit("last_name", 0, 1, offsetof(RealUser_Object, last_name), NULL),
        Property_SInit("language_code", 0, 0, offsetof(RealUser_Object, language_code), NULL),
        Property_ListEnd
};


static BotUser_Object *BotUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void BotUser_Dealloc(BotUser_Object *self);

static PyObject *BotUser_Repr(BotUser_Object *self);


static PyTypeObject BotUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotUser",
        .tp_basicsize = sizeof(BotUser_Object),
        .tp_new = (newfunc) BotUser_New,
        .tp_dealloc = (destructor) BotUser_Dealloc,
        .tp_base = &User_Type,
        .tp_repr = (reprfunc) &BotUser_Repr
};

static Property_Object BotUser_Meta[] = {
        Property_SInit("username", 0, 0, offsetof(BotUser_Object, common.username), NULL),
        Property_ListEnd
};

static BotUser_Object *BotSelfUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs);

static void BotSelfUser_Dealloc(BotSelfUser_Object *self);

static PyObject *BotSelfUser_Repr(BotSelfUser_Object *self);

static PyTypeObject BotSelfUser_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tgbot_compose.api_types.BotSelfUser",
        .tp_basicsize = sizeof(BotSelfUser_Object),
        .tp_new = (newfunc) BotSelfUser_New,
        .tp_dealloc = (destructor) BotSelfUser_Dealloc,
        .tp_base = &BotUser_Type,
        .tp_repr = (reprfunc) BotSelfUser_Repr
};


static Property_Object BotSelfUser_Meta[] = {
        Property_SInit("can_join_groups", 0, 0, offsetof(BotSelfUser_Object, can_join_groups), NULL),
        Property_SInit("can_read_all_group_messages", 0, 0, offsetof(BotSelfUser_Object, can_read_all_group_messages), NULL),
        Property_SInit("supports_inline_queries", 0, 0, offsetof(BotSelfUser_Object, supports_inline_queries), NULL),
        Property_ListEnd
};

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
    Py_XDECREF(self->common.id);
    Py_XDECREF(self->common.first_name);
    Py_XDECREF(self->common.username);
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
            self->common.id,
            self->common.username,
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
    Py_XDECREF(self->common.id);
    Py_XDECREF(self->common.first_name);
    Py_XDECREF(self->common.username);
    Py_TYPE(self)->tp_free(self);
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

static BotUser_Object *BotSelfUser_New(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    ifNullRet(kwargs = Parse_ArgsKwargsToDict(args, kwargs));

    ifNullRet(check_is_bot(&BotSelfUser_Type, kwargs, Py_False));

    return (BotUser_Object *) Parse_DictToProperties(kwargs, ignore_names, &BotSelfUser_Type, 0, 3, User_Meta, BotUser_Meta, BotSelfUser_Meta);
}

static void BotSelfUser_Dealloc(BotSelfUser_Object *self)
{
    Py_XDECREF(self->common.common.id);
    Py_XDECREF(self->common.common.first_name);
    Py_XDECREF(self->common.common.username);
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
            self->common.common.id,
            self->common.common.username,
            self->common.common.first_name,
            (parsePyBool(self->can_join_groups) ? " in groups" : ""),
            (parsePyBool(self->can_read_all_group_messages) ? " can read all" : ""),
            (parsePyBool(self->supports_inline_queries) ? " with inline" : "")
    );
}

FileInitFunction(User)
{
    Property_RInit(User_Type, User_Meta, 0, PyLong_Type);
    Property_RInit(User_Type, User_Meta, 1, PyUnicode_Type);
    Property_RInit(User_Type, User_Meta, 2, PyUnicode_Type);

    Property_RInit(RealUser_Type, RealUser_Meta, 0, PyUnicode_Type);
    Property_RInit(RealUser_Type, RealUser_Meta, 1, PyUnicode_Type);

    Property_RInit(BotUser_Type, BotUser_Meta, 0, PyUnicode_Type);

    Property_RInit(BotSelfUser_Type, BotSelfUser_Meta, 0, PyBool_Type);
    Property_RInit(BotSelfUser_Type, BotSelfUser_Meta, 1, PyBool_Type);
    Property_RInit(BotSelfUser_Type, BotSelfUser_Meta, 2, PyBool_Type);

    clsProps(User_Type, User_Meta);
    clsProps(RealUser_Type, RealUser_Meta);
    clsProps(BotUser_Type, BotUser_Meta);
    clsProps(BotSelfUser_Type, BotSelfUser_Meta);

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