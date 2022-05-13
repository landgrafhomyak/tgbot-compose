from collections.abc import Mapping, Iterable
import warnings


class export:
    __slots__ = "__func"

    def __init__(self, func):
        self.__func = func

    @property
    def __func__(self):
        return self.__func


class _callback:
    pass


class _exported_function:
    __slots__ = "__owner", "__function"

    @property
    def __func__(self):
        return self.__function

    @property
    def owner(self):
        return self.__owner

    def __init__(self, owner, func):
        self.__owner = owner
        self.__function = func


def _calc_mro(*bases):
    concurrent = dict()
    for mro in bases:
        for c, dc in enumerate(mro):
            concurrent.setdefault(dc, set()).update(set(mro[:c]))
    return tuple(c for c, l in sorted(concurrent.items(), key=lambda t: len(t[1])))


class component_storage:
    __slots__ = "__component", "__bot", "__fields"

    def __new__(cls, component, bot):
        self = super(cls, cls).__new__(cls)
        self.__component = component
        self.__bot = bot
        self.__fields = dict()
        return self

    def __getattribute__(self, item):
        if type(item) is not str:
            raise TypeError
        if item in self.__component._fields:
            try:
                return self.__fields[item]
            except KeyError:
                raise AttributeError(item)
        if item in self.__component._namespace:
            return self.__component._namespace[item].__get__(self)
        if item in self.__bot._export_table:
            return self.__bot._export_table[item]._get()
        raise AttributeError(item)

    def __setattr__(self, key, value):
        if type(key) is not str:
            raise TypeError
        if key in self.__component._fields:
            if not isinstance(value, self.__component._fields[key]):
                raise TypeError(f"Field {key !r} must be {self.__component._fields[key] !r}")
            try:
                self.__fields[key] = value
            except KeyError:
                raise AttributeError(key)
        if key in self.__component._namespace:
            return self.__component._namespace[key].__set__(self, value)
        if key in self.__bot._export_table:
            return self.__bot._export_table[key]._set(value)
        raise AttributeError(key)

    def __delattr__(self, name):
        if type(name) is not str:
            raise TypeError
        if name in self.__component._fields:
            raise TypeError(f"Field {name !r} is not deletable")
        if name in self.__component._namespace:
            return self.__component._namespace[name].__del__(self)
        if name in self.__bot._export_table:
            return self.__bot._export_table[name]._det()
        raise AttributeError(name)


class component:
    __slots__ = "__exports", "__imports", "__name", "__qualname", "__mro", "__cls", "__namespace"

    def __new__(cls, title, dependencies, namespace):
        if not isinstance(title, str):
            raise TypeError("Name must be str")
        name = str(title)
        if not isinstance(dependencies, Iterable):
            raise TypeError("Dependencies must be iterable")
        dependencies = tuple(dependencies)
        if not isinstance(namespace, Mapping):
            raise TypeError("Namespace must support mapping protocol")

        fields_raw = namespace.get("__annotations__", dict())
        if not isinstance(fields_raw, Mapping):
            raise TypeError("__annotations__ must support mapping protocol")
        fields = dict()
        for name, typ in fields_raw.items():
            if not isinstance(name, str):
                raise TypeError("Field name in must be string")
            if not hasattr(typ, "__instancecheck__"):
                raise TypeError("Field type must have __instancecheck__")
            fields[str(name)] = typ

        if "__slots__" in namespace:
            warnings.warn("Using __slots__ in tgbot_compose component", DeprecationWarning, stacklevel=2)
        slots = namespace.get("__slots__", ())
        if not isinstance(slots, Iterable):
            raise TypeError("__slots__ must be iterable")
        if isinstance(slots, str):
            slots = (slots,)
        else:
            slots = tuple(slots)

        for slot in slots:
            if not isinstance(slot, str):
                raise TypeError("Slot name in must be string")
            slot = str(slot)
            if slot in fields:
                warnings.warn(f"Duplication symbol {slot !r} in __slots__ and __annotations__", UserWarning, stacklevel=2)
            fields[slot] = object

        for name in fields.keys():
            if not name.isidentifier():
                raise TypeError(f"Invalid field name {name !r}")

        exports = dict()
        local_dict = dict()
        callbacks = set()
        for name, member in namespace.items():
            if not isinstance(name, str):
                raise TypeError("Member name must be str")
            name = str(name)
            if type(member) is export:
                exports[name] = member.__func__
            elif isinstance(member, _callback):
                callbacks.add(member)
            else:
                local_dict[name] = member

        for dependency in dependencies:
            if not type(dependency) is component:
                raise TypeError("Each dependency must be component")

        mro = _calc_mro(*map(lambda d: d.__mro, dependencies))

        imports = dict()
        for dependency in reversed(mro):
            imports.update(dependency.__exports)

        intersection = set(imports.keys()) & set(local_dict.keys())
        if intersection:
            raise TypeError(f"Non-export overriding of this symbols: {intersection !r}")
        del intersection

        qualname = namespace.get("__qualname__", name)
        if not isinstance(qualname, str):
            raise TypeError("__qualname__ must be str")
        qualname = str(qualname)

        self = super(cls, cls).__new__(cls)
        self.__name = title
        self.__imports = imports
        self.__exports = imports | {name: _exported_function(self, func) for name, func in exports.items()}
        self.__qualname = qualname
        self.__mro = (self,) + mro
        self.__cls = type(
            f"{component_storage.__name__}${title}",
            (component_storage,),
            {"__qualname__": f"{component_storage.__qualname__}${title}", "__slots__": fields.keys()}
        )
        self.__namespace = local_dict | {n: self.__cls.__dict__[n] for n in fields.keys()}

        return self

    def __repr__(self):
        return f"<component meta name={self.__qualname !r}>"

    @property
    def __name__(self):
        return self.__name

    def qualname_getter(self):
        return self.__qualname

    @property
    def __dependencies__(self):
        return self.__mro

    @property
    def import_names(self):
        return set(self.__imports.keys())

    @property
    def export_names(self):
        return set(self.__exports.keys())

    def __getattribute__(self, item):
        if item == "__qualname__":
            return self.__qualname
        else:
            return super(component, self).__getattribute__(item)

    @property
    def _namespace(self):
        return self.__namespace

    @property
    def _fields(self):
        return self.__fields


class bot:
    __next_id = 0
    __slots__ = "__component"

    def __new__(cls, *args):
        raise TypeError("Use bot[...] instead of constructor")

    def __class_getitem__(cls, item):
        if type(item) is component:
            item = (item,)
        if not isinstance(item, Iterable):
            raise TypeError("Components list must be iterable")
        self = super.__new__(cls)
        self.__component = component
        return self

    @property
    def globals(self):
        return self.__component.export_names

    @property
    def components(self):
        return self.__component.__dependencies__