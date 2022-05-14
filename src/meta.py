from asyncio import AbstractEventLoop
from collections.abc import Mapping, Iterable
import inspect
from typing import Type, Any, NoReturn
import warnings

from .backend import AbstractBackend


class export:
    __slots__ = "__func"

    __func: Any

    def __new__(cls: Type["export"], func: Any, /) -> "export":
        self: "export" = super(export, cls).__new__(cls)
        self.__func = func
        return self

    @property
    def __func__(self: "export", /) -> Any:
        return self.__func

    @property
    def __wrapped__(self: "export", /) -> Any:
        return self.__func

    def __repr__(self: "export", /) -> str:
        return f"<symbol {self.__func !r} marked for export>"


class _exported_symbol:
    __slots__ = "__owner", "__func", "__name"

    __owner: "component"
    __name: str
    __func: Any

    @property
    def __func__(self: "_exported_symbol", /) -> Any:
        return self.__func

    @property
    def owner(self: "_exported_symbol", /) -> "component":
        return self.__owner

    def __new__(cls: Type["_exported_symbol"], name: str, owner: "component", func: Any, /) -> "_exported_symbol":
        self: "_exported_symbol" = super(_exported_symbol, cls).__new__(cls)
        self.__name = name
        self.__owner = owner
        self.__func = func
        return self

    @property
    def __wrapped__(self: "_exported_symbol", /) -> Any:
        return self.__func

    def __repr__(self: "_exported_symbol", /) -> str:
        return f"<exported symbol {self.__name !r} of {self.__owner.__qualname__ !r}>"

    @property
    def __name__(self: "_exported_symbol", /) -> str:
        return self.__name


def _calc_mro(*bases: tuple["component"]) -> tuple["component", ...]:
    concurrent = dict()
    for mro in bases:
        for c, dc in enumerate(mro):
            concurrent.setdefault(dc, set()).update(set(mro[:c]))
    return tuple(c for c, l in sorted(concurrent.items(), key=lambda t: len(t[1])))


_default_value = object()


class _component_super_layer:
    __slots__ = "__component", "__table"

    __component: "component"
    __table: dict[str, "_bound_exported_symbol"]

    def __new__(cls: Type["_component_super_layer"], __component: "component", __table: dict[str, "_bound_exported_symbol"], /) -> "_component_super_layer":
        self: "_component_super_layer" = super(_component_super_layer, cls).__new__(cls)
        _component_super_layer.__component.__set__(self, __component)
        _component_super_layer.__table.__set__(self, __table)
        return self

    def __getattribute__(self: "_component_super_layer", item: str, /) -> Any:
        if type(item) is not str:
            raise TypeError(f"Attribute name must be str, got {type(item).__qualname__ !r}")

        i: dict[str, "_bound_exported_symbol"] = _component_super_layer.__table.__get__(self)

        try:
            s: "_bound_exported_symbol" = i[item]
        except KeyError:
            return AttributeError
        else:
            return s._get()

    def __setattr__(self: "_component_super_layer", key: str, value: Any, /) -> None:
        if type(key) is not str:
            raise TypeError(f"Attribute name must be str, got {type(key).__qualname__ !r}")

        i: dict[str, "_bound_exported_symbol"] = _component_super_layer.__table.__get__(self)

        try:
            s: "_bound_exported_symbol" = i[key]
        except KeyError:
            raise AttributeError(key)
        else:
            return s._set(value)

    def __delattr__(self: "_component_super_layer", name: str, /) -> None:
        if type(name) is not str:
            raise TypeError(f"Attribute name must be str, got {type(name).__qualname__ !r}")

        i: dict[str, "_bound_exported_symbol"] = _component_super_layer.__table.__get__(self)

        try:
            s: "_bound_exported_symbol" = i[name]
        except KeyError:
            raise AttributeError(name)
        else:
            return s._del()

    def __repr__(self: "_component_super_layer", /) -> str:
        return f"<super layer of component {_component_super_layer.__component.__get__(self).__qualname__ !r} at 0x{hex(id(self))[2:].zfill(16)}>"

    def get_component(self: "_component_super_layer", /) -> "component":
        return _component_super_layer.__component.__get__(self)


_component_super_layer_get_component = _component_super_layer.get_component
del _component_super_layer.get_component


class _component_instance:
    __slots__ = "__component", "__fields", "__bot", "__super"

    __component: "component"
    __fields: dict[str, type]
    __bot: "_bot_instance"
    __super: _component_super_layer

    def __new__(cls: Type["_component_instance"], __component: "component", __bot: "_bot_instance", __imports: dict[str, "_bound_exported_symbol"], /) -> "_component_instance":
        self: "_component_instance" = super(_component_instance, cls).__new__(cls)
        _component_instance.__component.__set__(self, __component)
        _component_instance.__fields.__set__(self, dict())
        _component_instance.__bot.__set__(self, __bot)
        _component_instance.__super.__set__(self, _component_super_layer(__component, __imports))
        return self

    def __getattribute__(self: "_component_instance", item: str, /) -> Any:
        if type(item) is not str:
            raise TypeError(f"Attribute name must be str, got {type(item).__qualname__ !r}")

        c: "component" = _component_instance.__component.__get__(self)
        f: dict[str, Any] = _component_instance.__fields.__get__(self)
        e: dict[str, "_bound_exported_symbol"] = _component_instance.__bot.__get__(self)._export_table

        if item == "__class__":
            return _component_instance
        elif item in c._fields:
            try:
                return f[item]
            except KeyError:
                raise AttributeError(item)
        elif item in c._namespace:
            return c._namespace[item].__get__(self)
        elif item in e:
            return e[item]._get()
        else:
            raise AttributeError(item)

    def __setattr__(self: "_component_instance", key: str, value: Any, /) -> None:
        if type(key) is not str:
            raise TypeError(f"Attribute name must be str, got {type(key).__qualname__ !r}")

        c: "component" = _component_instance.__component.__get__(self)
        f: dict[str, Any] = _component_instance.__fields.__get__(self)
        e: dict[str, "_bound_exported_symbol"] = _component_instance.__bot.__get__(self)._export_table

        if key in c._fields:
            if not isinstance(value, c._fields[key]):
                raise TypeError(f"Field {key !r} must be {c._fields[key] !r}")
            try:
                f[key] = value
            except KeyError:
                raise AttributeError(key)
        elif key in c._namespace:
            c._namespace[key].__set__(self, value)
        elif key in e:
            e[key]._set(value)
        else:
            raise AttributeError(key)

    def __delattr__(self: "_component_instance", name: str, /) -> None:
        if type(name) is not str:
            raise TypeError(f"Attribute name must be str, got {type(name).__qualname__ !r}")

        c: "component" = _component_instance.__component.__get__(self)
        e: dict[str, "_bound_exported_symbol"] = _component_instance.__bot.__get__(self)._export_table

        if name in c._fields:
            raise TypeError(f"Field {name !r} is not deletable")
        elif name in c._namespace:
            return c._namespace[name].__del__(self)
        elif name in e:
            return e[name]._del()
        else:
            raise AttributeError(name)

    def __repr__(self: "_component_instance", /) -> str:
        return f"<instance of component {_component_instance.__component.__get__(self).__qualname__ !r} at 0x{hex(id(self))[2:].zfill(16)}>"

    def __instancecheck__(self: "component", instance: "_component_instance", /) -> bool:
        return type(instance) is _component_instance and _component_instance.__component.__get__(instance) is self

    instancecheck = __instancecheck__
    del __instancecheck__

    def imports(instance: "_component_instance" = _default_value, /) -> "_component_super_layer":
        if instance is _default_value:
            for instance in inspect.currentframe().f_back.f_locals.values():
                break
            else:
                raise RuntimeError("Can't obtain component instance")
        if type(instance) is not _component_instance:
            raise TypeError("Component instance has wrong type")
        return _component_instance.__super.__get__(instance)

    def component_of(instance: _component_super_layer | "_component_instance") -> "component":
        if type(instance) is _component_super_layer:
            return _component_super_layer_get_component(instance)
        elif type(instance) is _component_super_layer:
            return _component_instance.__component.__get__(instance)
        else:
            raise TypeError(f"Can't get component from {type(instance).__qualname__ !r}")


imports = _component_instance.imports
del _component_instance.imports
imports.__qualname__ = f"{__name__}.imports"

component_of = _component_instance.component_of
del _component_instance.component_of
component_of.__qualname__ = f"{__name__}.component_of"


class _callback:
    pass


class component:
    __slots__ = "__exports", "__imports", "__name", "__qualname", "__mro", "__fields", "__namespace"
    __exports: dict[str, "_exported_symbol"]
    __imports: dict[str, "_exported_symbol"]
    __name: str
    __qualname: str
    __mro: tuple["component", ...]
    __fields: dict[str, type]
    __namespace: dict[str, Any]

    def __new__(cls: Type["component"], title: str, dependencies: Iterable["component"], namespace: Mapping[str, Any], /) -> "component":
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

        self = super(component, cls).__new__(cls)
        self.__name = title
        self.__imports = imports
        self.__exports = imports | {name: _exported_symbol(name, self, func) for name, func in exports.items()}
        self.__qualname = qualname
        self.__mro = (self,) + mro
        self.__fields = fields
        self.__namespace = local_dict

        return self

    def __repr__(self: "component", /) -> str:
        return f"<component meta name={self.__qualname !r}>"

    @property
    def __name__(self: "component", /) -> str:
        return self.__name

    @property
    def dependencies(self: "component", /) -> tuple["component", ...]:
        return self.__mro

    @property
    def import_names(self: "component", /) -> set[str]:
        return set(self.__imports.keys())

    @property
    def export_names(self: "component", /) -> set[str]:
        return set(self.__exports.keys())

    def __getattribute__(self: "component", item: str, /) -> Any:
        if item == "__qualname__":
            return self.__qualname
        else:
            return super(component, self).__getattribute__(item)

    @property
    def _namespace(self: "component", /) -> dict[str, Any]:
        return self.__namespace

    @property
    def _fields(self: "component", /) -> dict[str, type]:
        return self.__fields

    @property
    def _exports(self: "component", /) -> dict[str, "_exported_symbol"]:
        return self.__exports

    @property
    def _imports(self: "component", /) -> dict[str, "_exported_symbol"]:
        return self.__imports

    __instancecheck__ = _component_instance.instancecheck
    del _component_instance.instancecheck


component.__instancecheck__.__qualname__ = f"{component.__qualname__}.instancecheck"


class bot:
    __slots__ = "__component"

    __next_id: int = 0
    __component: "component"

    def __new__(cls: Type["bot"], *args: "component") -> NoReturn:
        raise TypeError("Use bot[...] instead of constructor")

    def __class_getitem__(cls: Type["bot"], item: Iterable["component"] | component, /) -> "bot":
        if type(item) is component:
            item = (item,)
        if not isinstance(item, Iterable):
            raise TypeError("Components list must be iterable")
        self = super(bot, bot).__new__(cls)
        self.__component = component(f"{cls.__name__}${cls.__next_id}", item, {"__qualname__": f"{cls.__qualname__}${cls.__next_id}"})
        cls.__next_id += 1
        return self

    @property
    def global_names(self: "bot", /) -> set[str]:
        return self.__component.export_names

    @property
    def components(self: "bot", /) -> tuple["component", ...]:
        return self.__component.dependencies

    @property
    def _exports(self: "bot", /) -> dict[str, "_exported_symbol"]:
        return self.__component._exports

    def __repr__(self: "bot", /) -> str:
        return f"<bot type at 0x{hex(id(self))[2:].zfill(16)}>"

    def __instancecheck__(self: "bot", instance: "_bot_instance", /) -> bool:
        return type(instance) is _bot_instance and instance.type is self


class _bot_instance:
    __slots__ = "__export_table", "__instances", "__type", "__loop"

    __instances: dict["component", "_component_instance"]
    __export_table: dict[str, "_bound_exported_symbol"]
    __type: "bot"
    __loop: AbstractEventLoop

    def instantiate(__meta: "bot", __backend: AbstractBackend = ..., /, *, loop: AbstractEventLoop, **arguments: Any) -> "_bot_instance":
        self = super(_bot_instance, _bot_instance).__new__(_bot_instance)
        self.__type = __meta
        self.__loop = loop
        self.__instances = dict()
        for comp in reversed(__meta.components):
            self.__instances[comp] = _component_instance(comp, self, {name: _bound_exported_symbol(self.__instances[ef.owner], ef.__func__) for name, ef in comp._imports.items()})
        self.__export_table = {name: _bound_exported_symbol(self.__instances[ef.owner], ef.__func__) for name, ef in __meta._exports.items()}
        for comp in reversed(__meta.components):
            try:
                init = self.__instances[comp].__init__
            except AttributeError:
                pass
            else:
                init(loop=loop, **arguments)
        return self

    bot.instantiate = instantiate
    instantiate.__qualname__ = f"{bot.__qualname__}.instantiate"
    del instantiate

    @property
    def _export_table(self: "_bot_instance", /) -> dict[str, "_bound_exported_symbol"]:
        return self.__export_table

    def __repr__(self: "_bot_instance", /) -> str:
        return f"<bot instance at 0x{hex(id(self))[2:].zfill(16)}>"

    @property
    def type(self: "_bot_instance", /) -> bot:
        return self.__type

    @property
    def loop(self: "_bot_instance", /) -> AbstractEventLoop:
        return self.__loop


class _bound_exported_symbol:
    __slots__ = "__instance", "__func"

    __instance: _component_instance
    __func: Any

    def __new__(cls: Type["_bound_exported_symbol"], instance: "_component_instance", func: Any, /) -> "_bound_exported_symbol":
        self = super(_bound_exported_symbol, cls).__new__(cls)
        self.__instance = instance
        self.__func = func
        return self

    def _get(self: "_bound_exported_symbol", /) -> Any:
        return self.__func.__get__(self.__instance)

    def _set(self: "_bound_exported_symbol", v: Any) -> None:
        return self.__func.__set__(self.__instance, v)

    def _del(self: "_bound_exported_symbol", /) -> None:
        return self.__func.__del__(self.__instance)

    @property
    def __func__(self: "_bound_exported_symbol", /) -> Any:
        return self.__func

    @property
    def __wrapped__(self: "_bound_exported_symbol", /) -> Any:
        return self.__func

    @property
    def __self__(self: "_bound_exported_symbol", /) -> Any:
        return self.__instance

    def __repr__(self: "_exported_symbol", /) -> str:
        return f"<bound exported symbol {self.__name !r} of {self.__owner.__qualname__ !r}>"


def depends_on(component_or_bot, dependency, /) -> bool:
    if type(component_or_bot) is component:
        if type(dependency) is component:
            return component in component_or_bot.dependencies
        else:
            raise TypeError("Dependency is not a component")
    elif type(component_or_bot) is _component_instance or type(component_or_bot) is _component_super_layer:
        if type(dependency) is component:
            return dependency in component_of(component_or_bot).dependencies
        elif type(dependency) is component:
            return component_of(dependency) in component_of(component_or_bot).dependencies
        else:
            raise TypeError("Dependency is not a component")
    elif type(component_or_bot) is bot:
        if type(dependency) is component:
            return component in bot.components
        else:
            raise TypeError("Dependency is not a component")
    elif type(component_or_bot) is _bot_instance:
        if type(dependency) is component:
            return dependency in component_or_bot.type.components
        elif type(dependency) is component:
            return component_of(dependency) in component_or_bot.type.components
        else:
            raise TypeError("Dependency is not a component")
    else:
        raise TypeError("The object under inspecting is not bot or component")
