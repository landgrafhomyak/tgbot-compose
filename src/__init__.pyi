from .api_types import *

from typing import final, Generic, Type, TypeVar

_O = TypeVar("_O")
_T = TypeVar("_T")


@final
class _ro_getattr_property(Generic[_O, _T]):
    def __get__(self, instance: _O, owner=Type[_O] | None) -> _T: ...

    def __call__(self, instance: _O) -> _T: ...

    @property
    def required_type(self) -> Type[_O]: ...

    @property
    def __name__(self) -> str: ...

class ClassClosure:
    pass


class BotComponent(type):
    @property
    def dependencies(cls) -> tuple[BotComponent, ...]: ...


class BotMeta(type):
    @property
    def components(cls) -> tuple[BotComponent, ...]: ...


class ApplicationMeta(type):
    @property
    def bots(cls) -> tuple[BotMeta, ...]: ...


def component(cls: ClassClosure) -> BotComponent: ...


def bot(cls: ClassClosure) -> BotMeta: ...


def app(cls: ClassClosure) -> ApplicationMeta: ...


@property
def message(self) -> Message | None: ...


@property
def edited_message(self) -> EditedMessage | None: ...


@property
def channel_post(self) -> ChannelPost | None: ...


@property
def edited_channel_post(self) -> EditedChannelPost | None: ...


@property
def inline_query(self) -> InlineQuery | None: ...


@property
def chosen_inline_result(self) -> ChosenInlineResult | None: ...


@property
def callback_query(self) -> CallbackQuery | None: ...


@property
def shipping_query(self) -> ShippingQuery | None: ...


@property
def pre_checkout_query(self) -> PreCheckoutQuery | None: ...


@property
def poll(self) -> Poll | None: ...


@property
def poll_answer(self) -> PollAnswer | None: ...


@property
def my_chat_member(self) -> MyChatMember | None: ...


@property
def chat_member(self) -> ChatMember | None: ...


@property
def chat_join_request(self) -> ChatJoinRequest | None: ...
