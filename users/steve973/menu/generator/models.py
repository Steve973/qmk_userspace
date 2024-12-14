from dataclasses import dataclass, field
from enum import Enum
from typing import Optional, List, Union

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
class Operation:
    action: str
    inputs: Optional[List[InputConfig]] = None
    confirm: Optional[ConfirmConfig] = None
    result: Optional[ResultConfig] = None


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
