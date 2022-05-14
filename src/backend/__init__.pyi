from abc import abstractmethod
from asyncio import AbstractEventLoop


class AbstractBackend:
    @abstractmethod
    def __new__(cls, loop: AbstractEventLoop, token: str) -> AbstractBackend: ...
