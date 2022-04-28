from typing import ClassVar, final, Generic, Type, TypeVar, Literal, overload, Any, TypeAlias

_T = TypeVar("_T")
_I = TypeVar("_I")
_M = TypeVar("_M", Literal[False], Literal[True])
_JSON: TypeAlias = dict[str, _JSON] | list[_JSON] | str | bool | int | float


@final
class _property(property, Generic[_T, _I, _M]):
    def __get__(self, instance: _I, owner: Any, /) -> _T:
        ...

    @overload
    def __call__(self, /, instance: _I) -> None:
        ...

    if _M:
        def __set__(self, instance: _I, value: _T, /) -> None:
            ...

        @overload
        def __call__(self, /, instance: _I, value: _T) -> None:
            ...

        if isinstance(None, _T):
            def __delete__(self, instance: _I, /) -> None: ...

    @property
    def __name__(self, /) -> str:
        ...

    @property
    def value_type(self, /) -> _T:
        ...

    @property
    def instance_type(self, /) -> _I:
        ...

    @property
    def offset(self, /) -> int:
        ...

    @property
    def is_optional(self, /) -> bool:
        ...

    @property
    def is_mutable(self, /) -> bool:
        ...


@final
class User:
    @overload
    def __new__(cls: Any, json: _JSON, /) -> RealUser | BotUser: ...

    @overload
    def __new__(
            cls: Any, /, *,
            is_bot: Literal[False] = False,
            id: int,
            first_name: str,
            last_name: str | None = None,
            language_code: str | None = None
    ) -> RealUser: ...

    @overload
    def __new__(
            cls: Any, /, *,
            is_bot: Literal[True] = False,
            id: int,
            first_name: str,
            username: str,
    ) -> BotUser: ...

    @property
    @overload
    def is_bot(self: RealUser) -> Literal[False]: ...

    @property
    @overload
    def is_bot(self: BotUser) -> Literal[True]: ...

    @property
    @overload
    def is_bot(self: User) -> bool: ...

    @property
    def id(self: User) -> int: ...

    @property
    def first_name(self: User) -> str: ...

    @property
    def username(self: User) -> str | None: ...


@final
class RealUser(User):
    @overload
    def __new__(cls: Any, json: _JSON, /) -> RealUser: ...

    @overload
    def __new__(
            cls: Any, /, *,
            is_bot: Literal[False] = False,
            id: int,
            first_name: str,
            last_name: str | None = None,
            username: str | None = None,
            language_code: str | None = None
    ) -> RealUser: ...

    @property
    def is_bot(self: RealUser) -> Literal[False]: ...

    @property
    def last_name(self: RealUser) -> str | None: ...

    @property
    def language_code(self: RealUser) -> str | None: ...


@final
class BotUser(User):
    @overload
    def __new__(cls: Any, json: _JSON, /) -> BotUser: ...

    @overload
    def __new__(
            cls: Any, /, *,
            is_bot: Literal[True] = False,
            id: int,
            first_name: str,
            username: str,
    ) -> BotUser: ...

    @property
    def is_bot(self: BotUser) -> Literal[True]: ...

    @property
    def username(self: BotUser) -> str: ...


@final
class BotSelfUser(BotUser):
    @overload
    def __new__(cls: Any, json: _JSON, /) -> BotSelfUser: ...

    @overload
    def __new__(
            cls: Any, /, *,
            is_bot: Literal[False] = False,
            id: int,
            first_name: str,
            username: str,
            can_join_groups: bool,
            can_read_all_group_messages: bool,
            supports_inline_queries: bool
    ) -> BotSelfUser: ...

    @property
    def can_join_groups(self: BotSelfUser) -> bool: ...

    @property
    def can_read_all_group_messages(self: BotSelfUser) -> bool: ...

    @property
    def supports_inline_queries(self: BotSelfUser) -> bool: ...


@final
class Update:
    @property
    def update_id(self) -> int: ...

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


@final
class Message(Update):
    pass


@final
class EditedMessage(Update):
    pass


@final
class ChannelPost(Update):
    pass


@final
class EditedChannelPost(Update):
    pass


@final
class InlineQuery(Update):
    pass


@final
class ChosenInlineResult(Update):
    pass


@final
class CallbackQuery(Update):
    pass


@final
class ShippingQuery(Update):
    pass


@final
class PreCheckoutQuery(Update):
    pass


@final
class Poll(Update):
    pass


@final
class PollAnswer(Update):
    pass


@final
class MyChatMember(Update):
    pass


@final
class ChatMember(Update):
    pass


@final
class ChatJoinRequest(Update):
    pass
