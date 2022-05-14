from asyncio import AbstractEventLoop
from collections.abc import Mapping, Iterable
from typing import final, TypeVar, Generic, Any, Type

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
    def __dependencies__(self: component, /) -> tuple[component, ...]: ...

    @property
    def import_names(self: component, /) -> set[str]: ...

    @property
    def export_names(self: component, /) -> set[str]: ...


class bot:
    @classmethod
    def __class_getitem__(cls: Type[bot], item: component | Iterable[component], /) -> bot: ...

    @property
    def global_names(self: bot, /) -> set[str]: ...

    @property
    def components(self: bot, /) -> tuple[component, ...]: ...

    def instantiate(self: bot, __backend: AbstractBackend = ..., /, *, loop: AbstractEventLoop, **arguments: Any) -> bot_instance: ...

    def __repr__(self: bot, /) -> str: ...


class bot_instance:
    def __repr__(self: bot_instance, /) -> str: ...
