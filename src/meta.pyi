from asyncio import AbstractEventLoop
from collections.abc import Mapping, Iterable
from typing import final, TypeVar, Generic, Any

_T = TypeVar("_T")


@final
class export(Generic[_T]):
    def __new__(cls: export, func: _T) -> export: ...

    @property
    def __func__(self) -> _T: ...


@final
class component:
    def __new__(cls, title: str, dependencies: Iterable[component], namespace: Mapping[str, Any]) -> component: ...

    def __repr__(self) -> str: ...

    @property
    def __name__(self) -> str: ...

    @property
    def __qualname__(self) -> str: ...

    @property
    def __dependencies__(self) -> tuple[component, ...]: ...

    @property
    def import_names(self) -> set[str]: ...

    @property
    def export_names(self) -> set[str]: ...


class bot:
    def __class_getitem__(cls, item: component | Iterable[component]) -> bot: ...

    @property
    def globals(self) -> set[str]: ...

    @property
    def components(self) -> tuple[component, ...]: ...

    def instantiate(self, backend: Any, loop: AbstractEventLoop, **arguments): ...
