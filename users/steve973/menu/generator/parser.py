from pathlib import Path
import json
from typing import Any, Dict, Union
from .models import MenuItem, Operation, InputConfig, ConfirmConfig, ResultConfig, ResultMode, InputType, Conditions, FeatureRule, ValueRule, RuleGroup, MatchType


def parse_menu_config(json_path: Union[str, Path]) -> MenuItem:
    """Parse menu configuration from JSON file"""
    with open(json_path) as f:
        data = json.load(f)
    return parse_menu_item(data["main_menu"])


def parse_menu_item(data: Dict[str, Any]) -> MenuItem:
    """Parse single menu item and children"""
    if "label" not in data or "type" not in data:
        raise ValueError("Menu item missing required fields")

    # Handle conditions.feature_enabled shorthand
    conditions = None
    if "conditions.feature_enabled" in data:
        conditions = Conditions.from_shorthand(data["conditions.feature_enabled"])
    elif "conditions" in data:
        conditions = parse_conditions(data["conditions"])

    # Parse operation if present
    operation = None
    if "operation" in data:
        operation = parse_operation(data["operation"])

    # Parse children recursively
    children = []
    if "children" in data:
        children = [parse_menu_item(child) for child in data["children"]]

    return MenuItem(
        label=data["label"],
        label_short=data.get("label_short"),
        icon=data.get("icon"),
        shortcut=data.get("shortcut"),
        type=data["type"],
        help_text=data.get("help_text"),
        operation=operation,
        conditions=conditions,
        children=children
    )


def parse_operation(data: Dict[str, Any]) -> Operation:
    """Parse operation configuration"""

    input_configs = None
    if "input" in data:
        # Parse array of inputs
        input_configs = [parse_input_config(input_data)
                        for input_data in data["input"]]

    confirm_config = None
    if "confirm" in data:
        confirm_config = parse_confirm_config(data["confirm"])

    result_config = None
    if "result" in data:
        result_config = parse_result_config(data["result"])

    return Operation(
        action=data["action"],
        inputs=input_configs,  # Changed from input to inputs
        confirm=confirm_config,
        result=result_config
    )


def parse_input_config(data: Dict[str, Any]) -> InputConfig:
    """Parse input configuration"""
    try:
        input_type = InputType(data["type"])
    except ValueError:
        raise ValueError(f"Invalid input type: {data['type']}")
    return InputConfig(
        type=InputType(data["type"]),
        default=data["default"],
        prompt=data.get("prompt"),  # Make optional
        wrap=data.get("wrap", False),
        live_preview=data.get("live_preview", False),
        range=data.get("range"),
        options=data.get("options"),
        options_conditions=data.get("options.conditions"),
        display_values=data.get("display_values")
    )


def parse_confirm_config(data: Dict[str, Any]) -> ConfirmConfig:
    """Parse confirmation dialog configuration"""
    return ConfirmConfig(
        message=data["message"],
        timeout_sec=data.get("timeout_sec", 0),
        default=data.get("default", True),
        true_text=data.get("true_text", "Yes"),
        false_text=data.get("false_text", "No")
    )


def parse_result_config(data: Dict[str, Any]) -> ResultConfig:
    """Parse result display configuration"""
    return ResultConfig(
        message=data["message"],
        mode=ResultMode(data["mode"]),
        timeout_sec=data.get("timeout_sec", 0),
        ok_text=data.get("ok_text")
    )


def parse_conditions(data: Dict[str, Any]) -> Conditions:
    """Parse condition rules"""
    rules = []
    for rule_data in data["rules"]:
        if "feature_enabled" in rule_data:
            rules.append(FeatureRule(feature=rule_data["feature_enabled"]))
        elif "value_equals" in rule_data:
            value_data = rule_data["value_equals"]
            rules.append(ValueRule(
                variable=value_data["variable"],
                value=value_data["value"]
            ))
        elif "match" in rule_data:
            rules.append(parse_rule_group(rule_data))

    return Conditions(
        match=MatchType(data["match"]),
        rules=rules
    )


def parse_rule_group(data: Dict[str, Any]) -> RuleGroup:
    """Parse nested condition rule group"""
    return RuleGroup(
        match=MatchType(data["match"]),
        rules=[parse_conditions(r) for r in data["rules"]]
    )
