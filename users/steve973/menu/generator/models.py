from dataclasses import dataclass, field
from enum import Enum
from typing import Optional, List, Union

class DisplayElementType(Enum):
    MESSAGE = "message"
    INPUT = "input"
    SELECTION = "selection"
    LIST = "list"

@dataclass
class DisplayElement:
    type: DisplayElementType
    text: str
    is_selectable: bool = False
    selection_char: str = ">"

@dataclass
class DisplayContent:
    title: str
    elements: List[DisplayElement]

class ResultMode(Enum):
    TIMED = "timed"
    ACKNOWLEDGE = "acknowledge"

class InputType(Enum):
    RANGE = "range"
    OPTIONS = "options"
    CUSTOM = "custom"

@dataclass
class InputConfig:
    type: InputType
    default: Union[str, int]
    prompt: Optional[str] = None
    wrap: bool = False
    live_preview: bool = False
    range: Optional[str] = None
    options: Optional[List[str]] = None
    options_conditions: Optional[dict] = None
    display_values: Optional[List[str]] = None
    title: str = "Input"
    timeout_sec: int = 5

@dataclass
class ConfirmConfig:
    message: str
    title: str = "Confirm"
    default: bool = True
    true_text: str = "OK"
    false_text: str = "Cancel"
    timeout_sec: int = 5

@dataclass
class ResultConfig:
    message: str
    mode: ResultMode
    title: str = "Result"
    ok_text: str = "OK"
    timeout_sec: int = 5

@dataclass
class PreconditionConfig:
    handler: str
    message: str
    args: Optional[dict] = None
    title: str = "Precondition"
    ok_text: str = "OK"
    timeout_sec: int = 5

@dataclass
class PostconditionConfig:
    handler: str
    message: str
    args: Optional[dict] = None
    title: str = "Postcondition"
    ok_text: str = "OK"
    timeout_sec: int = 5

@dataclass
class Operation:
    action: str
    precondition: Optional[PreconditionConfig] = None
    inputs: Optional[List[InputConfig]] = None
    confirm: Optional[ConfirmConfig] = None
    result: Optional[ResultConfig] = None
    postcondition: Optional[PostconditionConfig] = None

    def get_display_content(self, phase: str) -> DisplayContent:
        if phase == "precondition" and self.precondition:
            return DisplayContent(
                title=self.precondition.title,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.precondition.message or "Precondition failed",
                        is_selectable=False
                    ),
                    DisplayElement(
                        type=DisplayElementType.SELECTION,
                        text=self.precondition.ok_text,
                        is_selectable=True
                    )
                ]
            )

        elif phase == "input" and self.inputs:
            elements = []
            for idx, input in enumerate(self.inputs):
                if len(self.inputs) > 1:
                    elements.append(DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=f"Input {idx + 1} of {len(self.inputs)}",
                        is_selectable=False
                    ))
                elements.append(DisplayElement(
                    type=DisplayElementType.INPUT,
                    text=input.prompt,
                    is_selectable=True
                ))
            return DisplayContent(
                title=self.inputs[0].title,
                elements=elements
            )

        elif phase == "confirm" and self.confirm:
            return DisplayContent(
                title=self.confirm.title,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.confirm.message,
                        is_selectable=False
                    ),
                    DisplayElement(
                        type=DisplayElementType.SELECTION,
                        text=self.confirm.true_text,
                        is_selectable=True
                    ),
                    DisplayElement(
                        type=DisplayElementType.SELECTION,
                        text=self.confirm.false_text,
                        is_selectable=True
                    )
                ]
            )

        elif phase == "action":
            return DisplayContent(
                title="Action",
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text="Processing...",
                        is_selectable=False
                    )
                ]
            )

        elif phase == "result" and self.result:
            return DisplayContent(
                title=self.result.title,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.result.message,
                        is_selectable=False
                    ),
                    DisplayElement(
                        type=DisplayElementType.SELECTION,
                        text=self.result.ok_text,
                        is_selectable=True
                    )
                ]
            )

        elif phase == "postcondition" and self.postcondition:
            return DisplayContent(
                title=self.postcondition.title,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.postcondition.message,
                        is_selectable=False
                    ),
                    DisplayElement(
                        type=DisplayElementType.SELECTION,
                        text=self.postcondition.ok_text,
                        is_selectable=True
                    )
                ]
            )

        return None

class MenuType(Enum):
    ACTION = "action"
    SUBMENU = "submenu"
    DISPLAY = "display"

class MatchType(Enum):
    ALL = "all"
    ANY = "any"

@dataclass
class BaseRule:
    pass

@dataclass
class ValueRule:
    variable: str
    value: str
    type: str = "value_equals"  # default to value_equals

@dataclass
class ValueCompareRule:
    variable: str
    value: str
    operator: str  # will be one of: equals, not_equals, greater_than, less_than, greater_equals, less_equals
    type: str = "value_compare"

@dataclass
class RuleGroup(BaseRule):
    match: MatchType
    rules: List[Union[BaseRule, 'RuleGroup']]

@dataclass
class Conditions:
    match: MatchType
    rules: List[Union[BaseRule, RuleGroup]]

@dataclass
class MenuItem:
    label: str
    type: MenuType
    label_short: Optional[str] = None
    icon: Optional[str] = None
    shortcut: Optional[str] = None
    help_text: Optional[str] = None
    operation: Optional[Operation] = None
    conditions: Optional[Union[str, Conditions]] = None
    children: List['MenuItem'] = field(default_factory=list)
    screen_content: Optional[str] = None
    enabled_by: Optional[str] = None
    parent: Optional['MenuItem'] = None

    def __post_init__(self):
        self.type = MenuType(self.type)
        # Set parent refs
        for child in self.children:
            child.parent = self

    def get_display_content(self) -> DisplayContent:
        if self.type == MenuType.SUBMENU:
            elements = [
                DisplayElement(
                    type=DisplayElementType.LIST,
                    text=child.label,
                    is_selectable=True
                ) for child in self.children
            ]
            return DisplayContent(
                title=self.label,
                elements=elements
            )
