#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <utility>
#include <vector>
#include <stack>
#include <ios>
#include <set>
#include <algorithm>

#define NO_PYTHON

#include "serialization.h"

class ParseError : public std::runtime_error
{
public:
    size_t pos;

    ParseError(size_t pos, char const *message) noexcept:
            std::runtime_error(message),
            pos(pos)
    {}
};

class string
{
public:
    char const *start;
    size_t size;

    inline string(char const *start, size_t size) noexcept: start(start), size(size)
    {}

    template<size_t size>
    inline explicit string(char const (&literal)[size]) noexcept: start(literal), size(size - 1)
    {}

    inline string() noexcept: start(nullptr), size(0)
    {}

    inline explicit string(std::nullptr_t) noexcept: string()
    {}

    inline bool operator==(std::nullptr_t) const noexcept
    {
        return this->isNull();
    }

    inline bool isNull() const noexcept
    {
        return this->start == nullptr;
    }

    inline bool operator==(string const &other) const noexcept
    {
        return this->size == other.size && strncmp(this->start, other.start, this->size) == 0;
    }

    inline bool operator<(string const &other) const noexcept
    {
        return strncmp(this->start, other.start, std::max(this->size, other.size)) < 0;
    }
};

template<typename T>
static inline size_t pdiff(T const *lo, T const *hi)
{
    return (((uintptr_t) hi) - ((uintptr_t) lo)) / sizeof(T);
}

class MacroDef
{
public:
    const string name;
    const string args;
    const string definition;

    inline MacroDef(string name, string args, string definition) noexcept: name(name), args(args), definition(definition)
    {}

};

class IncludeDef
{
public:
    string path;

    inline IncludeDef(string path) noexcept: path(path)
    {}
};

class PropertyDef
{
public:
    string name;
    string accessor;
    string type_name;
    bool is_virtual;
    bool is_optional;
    string object_name;
    string converter;
    string getter;

    inline PropertyDef(
            string name,
            string accessor,
            string type_name,
            bool is_virtual,
            bool is_optional,
            string object_name,
            string validator = string(nullptr),
            string getter = string(nullptr)
    ) noexcept:
            name(name),
            accessor(accessor),
            type_name(type_name),
            is_virtual(is_virtual),
            is_optional(is_optional),
            object_name(object_name),
            converter(validator),
            getter(getter)
    {}
};

class ClassDef
{
public:
    string prefix;
    bool is_abstract;
    string type_name;
    string object_name;
    ClassDef const *base;
    std::vector<PropertyDef> properties;

    inline ClassDef(
            string prefix,
            bool is_abstract,
            string type_name,
            string object_name,
            ClassDef const *base,
            std::vector<PropertyDef> properties
    ) noexcept:
            prefix(prefix),
            is_abstract(is_abstract),
            type_name(type_name),
            object_name(object_name),
            base(base),
            properties(std::move(properties))
    {}
};

class Definitions
{
public:
    std::vector<IncludeDef> includes;
    std::vector<MacroDef> macros;
    std::vector<ClassDef const *> classes;

    inline Definitions(
            std::vector<IncludeDef> includes,
            std::vector<MacroDef> macros,
            std::vector<ClassDef const *> classes
    ) noexcept:
            includes(std::move(includes)),
            macros(std::move(macros)),
            classes(std::move(classes))
    {}

    inline Definitions() noexcept:
            includes(),
            macros(),
            classes()
    {}
};

class Tokenizer
{
private:
    char const *const raw;
    char const *p;

    explicit Tokenizer(char const *string) : raw(string), p(string)
    {}

    template<size_t string_size>
    inline bool assert_keyword(char const (&kw)[string_size]) noexcept
    {
        bool ret = std::strncmp(p, kw, string_size - 1) == 0;
        if (ret)
        { this->p += string_size - 1; }
        return ret;
    }

    inline void error [[noreturn]](char const *message)
    {
        throw ParseError(((uintptr_t) (this->p)) - ((uintptr_t) (this->raw)), message);
    }

    inline size_t skip_whitespaces() noexcept
    {
        size_t count = 0;
        while (*p == ' ')
        {
            count++;
        }
        return count;
    }


    inline string get_symbol()
    {
        char const *start = this->p;
        char c;
        while (true)
        {
            c = *(this->p);

            if (c == '_' || c == '.')
            { goto next; }

            if ('0' <= c && c <= '9')
            { goto next; }

            if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z')
            { goto next; }

            break;

            next:
            this->p++;
        }
        return string(start, pdiff(start, this->p));
    }

    template<size_t string_size>
    string get_property_member(char const (&keyword)[string_size])
    {
        if (!this->assert_keyword(keyword))
        { return string(); }
        if (!this->assert_keyword(" "))
        { this->error("After property member keyword must be whitespace"); }
        if (!this->assert_keyword("{"))
        { this->error("Missed { at start of property member implementation"); }
        auto stack = std::stack<char>();
        stack.push('{');
        bool in_string = false;
        bool was_slash = false;
        char const *start = this->p;
        while (!stack.empty())
        {
            switch (char c = *(this->p))
            {
                case '{':
                case '[':
                case '(':
                    if (in_string)
                    { break; }
                    stack.push(c);
                    break;
                case ')':
                case ']':
                case '}':
                    if (in_string)
                    { break; }
                    if (
                            (stack.top() == '(' && c != ')') ||
                            (stack.top() == '[' && c != ']') ||
                            (stack.top() == '{' && c != '}')
                            )
                    { this->error("Invalid closing brace, was expected another"); }
                    stack.pop();
                    break;
                case '"':
                    if (was_slash)
                    { break; }
                    in_string = !in_string;
                    break;
                case '\\':
                    if (!in_string || was_slash)
                    { break; }
                    was_slash = true;
                    goto NOT_CLEAR_SLASH;
            }
            was_slash = false;
            NOT_CLEAR_SLASH:
            this->p++;
        }
        string value(start, pdiff(start, this->p - 1));
        this->skip_whitespaces();
        if (!this->assert_keyword("\n"))
        { this->error("After property member implementation must be newline"); }
        return value;
    }


    inline bool isEnd() noexcept
    {
        return *(this->p) == 0;
    }

    inline string get_macro_args() noexcept
    {
        char const *start = this->p;
        if (!this->assert_keyword("("))
        { return string(nullptr); }
        while (*(this->p++) != ')')
        {}
        return string(start, pdiff(start, this->p));
    }

    inline string get_macro_body()
    {
        char const *start = this->p;
        char c;
        while (true)
        {
            switch (c = *this->p++)
            {
                case 0:
                case '\n':
                    goto EXIT_LOOP;
                case '\\':
                    if (*this->p != '\n')
                    { this->error("Missed newline after wrap in macro"); }
                    this->p++;
                    break;
            }
        }
        EXIT_LOOP:
        return string(start, pdiff(start, this->p));
    }

public:
    friend Definitions parse(char const *source);
};

Definitions parse(char const *source)
{
    auto t = Tokenizer(source);

    auto includes = std::vector<IncludeDef>();
    auto macros = std::vector<MacroDef>();
    auto classes = std::vector<ClassDef const *>();
    bool is_preprocessor_section_used = false;

    while (!t.isEnd())
    {
        size_t wsc = t.skip_whitespaces();

        if (!t.assert_keyword("#"))
        {
            if (wsc > 0 && !is_preprocessor_section_used)
            {
                t.error("Unexpected whitespaces at start of file");
            }
            break;
        }

        is_preprocessor_section_used = true;

        if (wsc > 0)
        { t.error("Unexpected indent before preprocessor declaration"); }

        if (t.assert_keyword("include"))
        {
            if (t.assert_keyword(" "))
            { t.error("Missed whitespace before include file path"); }
            continue;
        }
        if (t.assert_keyword("define"))
        {
            if (!t.assert_keyword(" "))
            { t.error("Missed whitespace after 'define' keyword"); }
            string name = t.get_symbol();
            string args = t.get_macro_args();
            if (!t.assert_keyword(" "))
            { t.error("Missed whitespace after macro signature"); }
            string body = t.get_macro_body();
            macros.emplace_back(name, args, body);
            continue;
        }
        t.error("Unexpected preprocessor directive");
    }
    if (!t.assert_keyword("\n"))
    { t.error("Missed newline after preprocessor section"); }

    while (!t.isEnd())
    {
        bool is_abstract = t.assert_keyword("abstract");
        if (is_abstract && !t.assert_keyword(" "))
        { t.error("After keyword 'abstract' must be whitespace"); }
        if (!t.assert_keyword("class"))
        { t.error("Missed keyword 'class' in class definition"); }
        if (!t.assert_keyword(" "))
        { t.error("After keyword 'class' must be whitespace"); }
        string class_name = t.get_symbol();
        if (!t.assert_keyword(" "))
        { t.error("After class name must be whitespace"); }
        if (!t.assert_keyword("["))
        { t.error("Missed [ after class name"); }
        string typeobj_name = t.get_symbol();
        string object_name = (!t.assert_keyword(",") ? string(nullptr) : (t.assert_keyword(" ") ? t.get_symbol() : (t.error("Before object struct name must whitespace"), string(nullptr))));
        if (!t.assert_keyword("]"))
        { t.error("Missed ] after typeobj (object) name"); }
        if (!t.assert_keyword(" "))
        { t.error("After class name (or object bind) must be whitespace"); }
        string base_n = (!t.assert_keyword("<-") ? string(nullptr) : (t.assert_keyword(" ") ? t.get_symbol() : (t.error("After <- must be whitespace"), string(nullptr))));
        auto properties = std::vector<PropertyDef>();
        ClassDef const *base = nullptr;
        if (!base_n.isNull())
        {
            for (auto base_i: classes)
            {
                if (base_n == base_i->prefix)
                {
                    base = base_i;
                    // properties.insert(properties.begin(), cls.properties.begin(), cls.properties.end());
                    goto BASE_FOUND;
                }
            }
            t.error("Base class not found");
            BASE_FOUND:
            if (!base_n.isNull() && !t.assert_keyword(" "))
            { t.error("After base class must be whitespace"); }
        }
        if (!t.assert_keyword("{"))
        { t.error("Missed { at start of class body"); }
        t.skip_whitespaces();
        if (!t.assert_keyword("\n"))
        { t.error("Missed newline at start of class body"); }
        while (true)
        {
            if (t.assert_keyword("    "))
            { goto PROPERTY; }
            t.skip_whitespaces();
            if (t.assert_keyword("\n"))
            { continue; }
            if (t.assert_keyword("}"))
            { break; }
            PROPERTY:
            bool is_virtual = false;
            if (!t.assert_keyword("@"))
            {
                if (!t.assert_keyword("~"))
                { t.error("Missed @ or ~ before property declaration"); }
                is_virtual = true;
            }
            if (!t.assert_keyword(" "))
            { t.error("After @ must be whitespace"); }
            string property_name = t.get_symbol();
            if (!t.assert_keyword(" "))
            { t.error("After property name must be whitespace"); }
            string property_custom_access = (!t.assert_keyword("[") ? property_name :
                                             [&]() {
                                                 string v = t.get_symbol();
                                                 if (!t.assert_keyword("]"))
                                                 { t.error("Missed ] after property accessor"); }
                                                 if (!t.assert_keyword(" "))
                                                 { t.error("After property accessor must be whitespace"); }
                                                 return v;
                                             }());
            if (!t.assert_keyword(":"))
            { t.error("Missed : between property name and type"); }
            if (!t.assert_keyword(" "))
            { t.error("After : must be whitespace"); }
            string property_type_name = t.get_symbol();
            bool is_optional = t.assert_keyword("?");
            string property_object_name = (t.assert_keyword("[") ? t.get_symbol() : string());
            if (!property_object_name.isNull() && !t.assert_keyword("]"))
            { t.error("Missed ] after object name"); }
            if (!t.assert_keyword("\n"))
            { t.error("After property type (or object) must be newline"); }
            if (!t.assert_keyword("        "))
            {
                properties.emplace_back(property_name, property_custom_access, property_type_name, is_virtual, is_optional, property_object_name);
                continue;
            }
            string validator = t.get_property_member("check");
            if (!validator.isNull())
            {
                if (!t.assert_keyword("        "))
                {
                    properties.emplace_back(property_name, property_custom_access, property_type_name, is_virtual, is_optional, property_object_name, validator);
                    continue;
                }
            }
            string getter = t.get_property_member("get");
            properties.emplace_back(property_name, property_custom_access, property_type_name, is_virtual, is_optional, property_object_name, validator, getter);
        }
        classes.push_back(new ClassDef(class_name, is_abstract, typeobj_name, object_name, base, properties));
        t.skip_whitespaces();
        if (!t.assert_keyword("\n"))
        {
            if (t.isEnd())
            { break; }
            t.error("After class implementation must be newline");
        }
        t.skip_whitespaces();
        if (!t.assert_keyword("\n"))
        {
            if (t.isEnd())
            { break; }
            t.error("After class implementation must be empty line");
        }
    }

    return Definitions(includes, macros, classes);
}

std::ostream &operator<<(std::ostream &os, string s)
{
    return os.write(s.start, std::streamsize(s.size));
}

static inline string tonopo(string s)
{
    if (s.isNull())
    { return string("PyObject"); }
    else
    { return s; }
}

template<size_t fun_type_size>
static inline std::ostream &fun_name(std::ostream &out, string prefix, char const (&type)[fun_type_size], string name)
{
    auto hash_string_cropped = std::string(prefix.start, prefix.size);
    return out << "_" << hash_string(hash_string_cropped.c_str()) << "_" << type << "_" << name;
}

class inherit_props
{
private:
    ClassDef const *cls;
    std::vector<PropertyDef>::const_iterator pos;
    std::vector<PropertyDef>::const_iterator end_pos;
    std::set<string> processed_keys;

public:
    class PropertyDefWithOwner : public PropertyDef
    {
    public:
        ClassDef const *owner;

        PropertyDefWithOwner(PropertyDef const *value, ClassDef const *owner) noexcept:
                PropertyDef(*value),
                owner(owner)
        {}
    };

    inherit_props(ClassDef const *cls) noexcept:
            cls(cls),
            pos(cls->properties.begin()),
            end_pos(cls->properties.end())
    {}

    inherit_props operator++()
    {
        while (this->cls != nullptr)
        {
            for (; this->pos != this->end_pos; this->pos++)
            {
                if (this->processed_keys.count(this->pos->name) > 0)
                { continue; }
                this->processed_keys.insert(this->pos->name);
                goto RET;
            }
            this->cls = this->cls->base;
            if (this->cls != nullptr)
            {
                this->pos = this->cls->properties.begin();
                this->end_pos = this->cls->properties.end();
            }
        }
        RET:
        return *this;
    }

    PropertyDefWithOwner operator*()
    {
        this->processed_keys.insert(this->pos->name);
        return PropertyDefWithOwner(&*(this->pos), this->cls);
    }

    inherit_props const &end() const noexcept
    {
        return *this;
    }

    inherit_props &begin() noexcept
    {
        return *this;
    }

    bool operator!=(inherit_props const &other)
    {
        return this->cls != nullptr;
    }
};


static void generate(Definitions const &defs, std::ostream &out)
{
    for (auto include: defs.includes)
    {
        out << "#include " << include.path << std::endl;
    }

    out << "#include \"serialization.h\"" << std::endl;
    out << "#define converter_success (1)" << std::endl;
    out << "#define converter_failed (0)" << std::endl;
    out << "#define converter_with_cleanup (Py_CLEANUP_SUPPORTED)" << std::endl;

    for (auto macro: defs.macros)
    {
        out << "#define " << macro.name << macro.args << " " << macro.definition << std::endl;
    }


    for (auto cls: defs.classes)
    {
        auto prefix_hash_source = std::string(cls->prefix.start, cls->prefix.size);
        uint64_t prefix_hash = hash_string(prefix_hash_source.c_str());

        for (auto property: cls->properties)
        {
            out << "static " << tonopo(property.object_name) << " const * ";
            fun_name(out, cls->prefix, "get", property.name);
            out << "(" << tonopo(cls->object_name) << " const * self, void *_)" << std::endl;
            if (!property.getter.isNull())
            { out << "{" << property.getter << "}" << std::endl; }
            else
            {
                out << "{" << std::endl;
                out << "    PyObject const *value = *(PyObject const **)(((uintptr_t)self) + offsetof(" << tonopo(cls->object_name) << ", " << property.accessor << "));" << std::endl;
                out << "    if (value == NULL)" << std::endl;
                if (property.is_optional)
                { out << "        Py_RETURN_NONE;" << std::endl; }
                else
                {
                    out << "    {" << std::endl;
                    out << "        PyErr_Format(PyExc_RuntimeError, \"Value of property `" << property.name << "` was lost\");" << std::endl;
                    out << "        return NULL;" << std::endl;
                    out << "    }" << std::endl;
                }
                out << "    else" << std::endl;
                out << "        return Py_NewRef(value);" << std::endl;
                out << "}" << std::endl;
            }
            out << "static inline int ";
            fun_name(out, cls->prefix, "converter", property.name) << "(" << tonopo(property.object_name) << " const * value, " << tonopo(property.object_name) << " **storage)" << std::endl;
            if (!property.converter.isNull())
            { out << "{" << property.converter << "}" << std::endl; }
            else
            {
                out << "{" << std::endl;
                out << "    if (value == NULL)" << std::endl;
                out << "    {" << std::endl;
                out << "        Py_XDECREF(*storage);" << std::endl;
                out << "        return 1;" << std::endl;
                out << "    }" << std::endl;
                if (property.is_optional)
                {
                    out << "    if (value == Py_None)" << std::endl;
                    out << "    {" << std::endl;
                    if (!property.is_virtual)
                    { out << "        *storage = NULL;" << std::endl; }
                    out << "        return 1;" << std::endl;
                    out << "    }" << std::endl;
                    out << "    else ";
                }
                else
                { out << "    "; }
                out << "if (PyType_IsSubtype(Py_TYPE(value), &" << property.type_name << "))" << std::endl;
                out << "    {" << std::endl;
                if (!property.is_virtual)
                {
                    out << "        *storage = Py_NewRef(value);" << std::endl;
                    out << "        return Py_CLEANUP_SUPPORTED;" << std::endl;
                }
                else
                {
                    out << "        return 1;" << std::endl;
                }
                out << "    }" << std::endl;
                out << "    PyErr_Format(PyExc_TypeError, \"`" << property.name << "` must be `%s` or its subclass, got `%s`\", " << property.type_name << ".tp_name, Py_TYPE(value)->tp_name);" << std::endl;
                out << "    return 0;" << std::endl;
                out << "}" << std::endl;
            }
        }
        out << "#define " << cls->prefix << "_GetSet_GENERATED \\" << std::endl;
        auto existing_properties = std::set<string>();

        for (auto property: cls->properties /* inherit_props(cls)*/)
        {
            out << "{\"" << property.name << "\", (getter)";
            fun_name(out, cls->prefix, "get", property.name) << ", NULL, NULL}, \\" << std::endl;
        }

        out << std::endl;
        if (!cls->is_abstract)
        {
            out << "static inline void " << cls->prefix << "_Dealloc_GENERATED(" << tonopo(cls->object_name) << " *self)" << std::endl;

            out << "{" << std::endl;
            for (auto property: inherit_props(cls))
            {
                if (!property.is_virtual)
                { out << "    Py_XDECREF(((" << property.owner->object_name << " *)self)->" << property.accessor << ");" << std::endl; }

            }
            out << "}" << std::endl;
            out << "static void ";
            fun_name(out, cls->prefix, "dealloc", string("")) << "(" << cls->object_name << " *self)" << std::endl;
            out << "{" << std::endl;
            out << "    " << cls->prefix << "_Dealloc_GENERATED(self);" << std::endl;
            out << "    Py_TYPE(self)->tp_free(self);" << std::endl;
            out << "}" << std::endl;
            out << "#define " << cls->prefix << "_Dealloc (";
            fun_name(out, cls->prefix, "dealloc", string("")) << ")" << std::endl;


            auto sorted_props = std::vector<inherit_props::PropertyDefWithOwner>();
            for (auto property: inherit_props(cls))
            {
                sorted_props.push_back(property);
            }
            std::sort(sorted_props.begin(), sorted_props.end(), [](PropertyDef a, PropertyDef b) { return a.is_optional < b.is_optional; });
            out << "static inline PyObject *" << cls->prefix << "_Init_GENERATED(" << cls->object_name << " *self, PyObject *dict)" << std::endl;
            out << "{" << std::endl;
            out << "    static char const *const kw_names[] = {";
            for (auto property: sorted_props)
            {
                out << "\"" << property.name << "\", ";
            }
            out << "NULL};" << std::endl;
            out << "    static PyObject *virtual_arg_storage;" << std::endl;
            out << "    PyObject *args = PyTuple_Pack(0);" << std::endl;
            out << "    if (args == NULL) return NULL;" << std::endl;
            out << "    if (!PyArg_ParseTupleAndKeywords(args, dict, \"";
            bool already_optional = false;
            for (auto property: sorted_props)
            {
                if (property.is_optional)
                {
                    if (!already_optional)
                    { out << "|"; }
                    already_optional = true;
                }
                out << "O&";
            }
            out << "\", kw_names, ";
            for (auto property: sorted_props)
            {
                fun_name(out, property.owner->prefix, "converter", property.name) << ", ";
                if (property.is_virtual)
                { out << "&virtual_arg_storage"; }
                else
                { out << "&(((" << property.owner->object_name << " *)self)->" << property.accessor << ")"; }
                out << ", ";
            }
            out << "0))" << std::endl;
            out << "    { Py_DECREF(args); return NULL; }" << std::endl;
            out << "    Py_DECREF(args);" << std::endl;
            out << "    return self;" << std::endl;
            out << "}" << std::endl;
            out << "static " << tonopo(cls->object_name) << " *";
            fun_name(out, cls->prefix, "new", string("_")) << "(PyTypeObject *cls, PyObject *args, PyObject **kwargs)" << std::endl;
            out << "{" << std::endl;
            out << "    PyObject *dict = parse_kwargs(args, kwargs);" << std::endl;
            out << "    if (dict == NULL) return NULL;" << std::endl;
            out << "    PyObject *self = cls->tp_alloc(cls, 0);" << std::endl;
            out << "    if (self == NULL) return NULL;" << std::endl;
            out << "    if (" << cls->prefix << "_Init_GENERATED(self, dict) == NULL)" << std::endl;
            out << "    { Py_DECREF(self); return NULL; }" << std::endl;
            out << "    return self;" << std::endl;
            out << "}" << std::endl;
            out << "#define " << cls->prefix << "_New (";
            fun_name(out, cls->prefix, "new", string("_")) << ")" << std::endl;
        }
    }
    out << "#undef converter_successful" << std::endl;
    out << "#undef converter_failed" << std::endl;
    out << "#undef converter_with_cleanup" << std::endl;


    for (auto macro: defs.macros)
    {
        out << "#undef " << macro.name << std::endl;
    }
}


int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        std::cerr << "No input and output files" << std::endl;
        return -1;
    }

    if (argc > 3)
    {
        std::cerr << "Too many arguments" << std::endl;
        return -2;
    }

    std::ifstream in(argv[1]);
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();
    Definitions data;
    try
    {
        data = parse(source.c_str());
    }
    catch (ParseError &err)
    {
        char const *line_start = source.c_str() + err.pos;
        size_t indent = 0;
        if (*line_start == '\n')
        {
            indent++;
            line_start--;
        }
        while (((uintptr_t) line_start) > ((uintptr_t) source.c_str()) && *line_start != '\n')
        {
            line_start--;
            indent++;
        }
        size_t line_no = std::count(source.c_str(), line_start, '\n') + 1;
        if (*line_start == '\n')
        {
            line_start++;
            line_no++;
        }
        else
        {
            indent++;
        }

        size_t line_size = 0;
        while (true)
        {
            switch (*(line_start + line_size))
            {
                case '0':
                case '\n':
                    goto exit_loop;
            }
            line_size++;
        }

        exit_loop:
        std::cerr << "Can't parse definitions at line " << line_no << std::endl;
        std::cerr.write(line_start, std::streamsize(line_size)) << std::endl;
        std::cerr << std::setfill(' ') << std::setw(std::streamsize(indent)) << '^' << std::endl;
        std::cerr << err.what() << std::endl;
        return 1;
    }
    in.close();

    std::ofstream out(argv[2]);
    generate(data, out);

    out.close();

    return 0;
}