#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <iomanip>
#include <utility>
#include <vector>
#include <stack>

class ParseError : public std::exception
{
public:
    size_t pos;

    ParseError(size_t pos, char const *message) noexcept:
            std::exception(message),
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
};

template<typename T>
static inline size_t pdiff(T const *lo, T const *hi)
{
    return (((uintptr_t) hi) - ((uintptr_t) lo)) / sizeof(T);
}

class MacroDef
{
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
    string type_name;
    bool is_optional;
    string object_name;
    string validator;
    string getter;

    inline PropertyDef(
            string name,
            string type_name,
            bool is_optional,
            string object_name,
            string validator = string(nullptr),
            string getter = string(nullptr)
    ) noexcept:
            name(name),
            type_name(type_name),
            is_optional(is_optional),
            object_name(object_name),
            validator(validator),
            getter(getter)
    {}
};

class ClassDef
{
public:
    string type_name;
    string object_name;
    string base;
    std::vector<PropertyDef> properties;

    inline ClassDef(
            string type_name,
            string object_name,
            string base,
            std::vector<PropertyDef> properties
    ) noexcept:
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
    std::vector<ClassDef> classes;

    inline Definitions(
            std::vector<IncludeDef> includes,
            std::vector<MacroDef> macros,
            std::vector<ClassDef> classes
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

            if (c == '_')
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

    friend Definitions parse(char const *source);

    inline bool isEnd()
    {
        return *(this->p) == 0;
    }

};

static Definitions parse(char const *source)
{
    auto t = Tokenizer(source);

    auto includes = std::vector<IncludeDef>();
    auto macros = std::vector<MacroDef>();
    auto classes = std::vector<ClassDef>();
    bool is_preprocessor_section_used = false;

    while (true)
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
            {
                t.error("Missed whitespace before include file path");
            }
        }
        if (t.assert_keyword("define"))
        {}
        t.error("Unexpected preprocessor directive");

        next:
        if (!t.assert_keyword("\n"))
        {
            t.error("Missed newline after preprocessor declaration");
        }

    }

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
        string object_name = (t.assert_keyword("[") ? t.get_symbol() : string());
        if (!object_name.isNull() && !t.assert_keyword("]"))
        { t.error("Missed ] after object name"); }
        if (!t.assert_keyword(" "))
        { t.error("After class name (or object bind) must be whitespace"); }
        string base = (!t.assert_keyword("<-") ? string(nullptr) : (t.assert_keyword(" ") ? t.get_symbol() : (t.error("After <- must be whitespace"), string(nullptr))));
        if (!base.isNull() && !t.assert_keyword(" "))
        { t.error("After base class must be whitespace"); }
        if (!t.assert_keyword("{"))
        { t.error("Missed { at start of class body"); }
        t.skip_whitespaces();
        if (!t.assert_keyword("\n"))
        { t.error("Missed newline at start of class body"); }
        auto properties = std::vector<PropertyDef>();
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
            if (!t.assert_keyword("@"))
            { t.error("Missed @ before property declaration"); }
            if (!t.assert_keyword(" "))
            { t.error("After @ must be whitespace"); }
            string property_name = t.get_symbol();
            if (!t.assert_keyword(" "))
            { t.error("After property name must be whitespace"); }
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
                properties.emplace_back(property_name, property_type_name, is_optional, property_object_name);
                continue;
            }
            string validator = t.get_property_member("check");
            if (!validator.isNull())
            {
                if (!t.assert_keyword("        "))
                {
                    properties.emplace_back(property_name, property_type_name, is_optional, property_object_name, validator);
                    continue;
                }
            }
            string getter = t.get_property_member("get");
            properties.emplace_back(property_name, property_type_name, is_optional, property_object_name, validator, getter);
        }
        classes.emplace_back(class_name, object_name, base, properties);
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


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cerr << "No input files" << std::endl;
        return -1;
    }

    if (argc > 2)
    {
        std::cerr << "Too many arguments" << std::endl;
        return -2;
    }

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();
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



    return 0;
}