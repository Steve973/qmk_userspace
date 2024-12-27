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

@dataclass
class ConfirmConfig:
    message: str
    timeout_sec: int
    default: bool = True
    true_text: str = "Yes"
    false_text: str = "No"

@dataclass
class ResultConfig:
    message: str
    mode: ResultMode
    timeout_sec: Optional[int] = None
    ok_text: Optional[str] = None


@dataclass
class PreconditionConfig:
    handler: str
    message: str
    args: Optional[dict] = None

@dataclass
class PostconditionConfig:
    handler: str
    message: str
    args: Optional[dict] = None

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
                title=self.action,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.precondition.message,
                        is_selectable=False
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
                title=self.action,
                elements=elements
            )

        elif phase == "confirm" and self.confirm:
            return DisplayContent(
                title=self.action,
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
            # Action phase might show progress or status
            return DisplayContent(
                title=self.action,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text="Processing...",
                        is_selectable=False
                    )
                ]
            )

        elif phase == "result" and self.result:
            elements = [
                DisplayElement(
                    type=DisplayElementType.MESSAGE,
                    text=self.result.message,
                    is_selectable=False
                )
            ]
            if self.result.mode == ResultMode.ACKNOWLEDGE:
                elements.append(DisplayElement(
                    type=DisplayElementType.SELECTION,
                    text=self.result.ok_text or "OK",
                    is_selectable=True
                ))
            return DisplayContent(
                title=self.action,
                elements=elements
            )

        elif phase == "postcondition" and self.postcondition:
            return DisplayContent(
                title=self.action,
                elements=[
                    DisplayElement(
                        type=DisplayElementType.MESSAGE,
                        text=self.postcondition.message,
                        is_selectable=False
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
class FeatureRule(BaseRule):
    feature: str

@dataclass
class ValueRule(BaseRule):
    variable: str
    value: str

@dataclass
class RuleGroup(BaseRule):
    match: MatchType
    rules: List[Union[BaseRule, 'RuleGroup']]

@dataclass
class Conditions:
    match: MatchType
    rules: List[Union[BaseRule, RuleGroup]]

    @classmethod
    def from_shorthand(cls, feature: str) -> 'Conditions':
        return cls(
            match=MatchType.ALL,
            rules=[FeatureRule(feature=feature)]
        )

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
    parent: Optional['MenuItem'] = None

    def __post_init__(self):
        self.type = MenuType(self.type)
        # Handle shorthand conditions
        if isinstance(self.conditions, str):
            self.conditions = Conditions.from_shorthand(self.conditions)
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
