from asyncio import AbstractEventLoop
from collections.abc import Mapping, Iterable
from typing import final, TypeVar, Generic, Any, Type, overload

from .backend import AbstractBackend

_T = TypeVar("_T")


@final
class export(Generic[_T]):
    @classmethod
    def __new__(cls: Type[export], func: _T, /) -> export: ...

    @property
    def __func__(self: export, /) -> _T: ...

    @property
    def __wrapped__(self: export, /) -> _T: ...

    def __repr__(self: export, /) -> str: ...


@final
class component:
    @classmethod
    def __new__(cls: Type[component], title: str, dependencies: Iterable[component], namespace: Mapping[str, Any], /) -> component: ...

    def __repr__(self: component, /) -> str: ...

    @property
    def __name__(self: component, /) -> str: ...

    @property
    def __qualname__(self: component, /) -> str: ...

    @property
    def dependencies(self: component, /) -> tuple[component, ...]: ...

    @property
    def import_names(self: component, /) -> set[str]: ...

    @property
    def export_names(self: component, /) -> set[str]: ...

    def __instancecheck__(self: component, instance: _component_instance, /) -> bool: ...


@final
class bot:
    @classmethod
    def __class_getitem__(cls: Type[bot], item: component | Iterable[component], /) -> bot: ...

    @property
    def global_names(self: bot, /) -> set[str]: ...

    @property
    def components(self: bot, /) -> tuple[component, ...]: ...

    def instantiate(self: bot, __backend: AbstractBackend = ..., /, *, loop: AbstractEventLoop, **arguments: Any) -> _bot_instance: ...

    def __repr__(self: bot, /) -> str: ...

    def __instancecheck__(self: bot, instance: _bot_instance, /) -> bool: ...


@final
class _bot_instance:
    def __repr__(self: _bot_instance, /) -> str: ...

    @property
    def type(self: _bot_instance, /) -> bot: ...

    @property
    def loop(self: _bot_instance, /) -> AbstractEventLoop: ...


@final
class _component_instance:
    def __getattribute__(self: _component_instance, item: str, /) -> Any: ...

    def __setattr__(self: _component_instance, key: str, value: Any, /) -> None: ...

    def __delattr__(self: _component_instance, name: str, /) -> None: ...

    def __repr__(self: _component_instance, /) -> str: ...


@final
class _component_super_layer:
    def __getattribute__(self: _component_super_layer, item: str, /) -> Any: ...

    def __setattr__(self: _component_super_layer, key: str, value: Any, /) -> None: ...

    def __delattr__(self: _component_super_layer, name: str, /) -> None: ...

    def __repr__(self: _component_super_layer, /) -> str: ...


@overload
def imports() -> _component_super_layer: ...


@overload
def imports(instance: _component_instance, /) -> _component_super_layer: ...


def component_of(instance: _component_instance | _component_super_layer, /) -> component: ...


@overload
def depends_on(component_or_bot: bot | component, dependency: component, /) -> bool: ...


@overload
def depends_on(component_or_bot: _bot_instance | _component_instance | _component_super_layer, dependency: component | _component_instance, /) -> bool: ...
