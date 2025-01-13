from typing import Any, Dict, List, Set, Tuple, Union, Optional
from pathlib import Path
import json
from typing import Any, Dict, Union
from .models import (
    MenuItem, Operation, PreconditionConfig, InputConfig, ConfirmConfig,
    ResultConfig, PostconditionConfig, ResultMode, InputType, Conditions,
    ValueRule, RuleGroup, MatchType
)


def parse_menu_config(json_paths: List[Path], enabled_features: Set[str]) -> Tuple[MenuItem, Set[str]]:
    """Parse menu configuration and collect all action names"""
    action_names = set()

    merged_data = {
        "main_menu": {
            "label": "Main Menu",
            "label_short": "Main",
            "icon": "NONE",
            "type": "submenu",
            "children": []
        }
    }

    # Merge items from all JSON files
    for path in json_paths:
        with open(path) as f:
            items = json.load(f)
            # If it's wrapped in main_menu or children, get the actual items
            if isinstance(items, dict):
                items = items.get("children", []) if "children" in items else items.get("main_menu", {}).get("children", [])
            items = sorted(items, key=lambda x: x["label"])
            merged_data["main_menu"]["children"].extend(
                filter_menu_items(items, enabled_features)
            )

    root = parse_menu_item(merged_data["main_menu"], action_names, enabled_features)
    return root, action_names

def filter_menu_items(items: List[Dict], enabled_features: Set[str]) -> List[Dict]:
    """Filter menu items based on feature conditions"""
    filtered = []
    for item in items:
        include, enabled_by = should_include_item(item, enabled_features)
        if include:
            if "children" in item:
                item["children"] = filter_menu_items(item["children"], enabled_features)
            if enabled_by:
                item["enabled_by"] = enabled_by
            filtered.append(item)
    return filtered

def should_include_item(item: Dict, enabled_features: Set[str]) -> Tuple[bool, Optional[str]]:
    """Check if an item should be included based on feature conditions"""
    # Check for feature enable condition
    if "conditions.feature_enabled" in item:
        feature = item["conditions.feature_enabled"]
        is_enabled = feature in enabled_features
        return (feature in enabled_features, feature)

    # Check for value conditions
    if "conditions" in item:
        conditions = item["conditions"]
        if "match" in conditions and "rules" in conditions:
            rules = conditions["rules"]
            match_type = conditions["match"]

            # Keep only value conditions
            value_rules = []
            for rule in rules:
                value_rules.append(rule)

            # Update conditions to only include value rules
            if value_rules:
                conditions["rules"] = value_rules
            else:
                del item["conditions"]

    # If no conditions or only value conditions, include the item
    return (True, None)

def merge_menu_data(target: Dict, source: Dict, path: str = ""):
    """Merge menu structures with override protection"""
    for key, value in source.items():
        current_path = f"{path}/{key}" if path else key

        if key in target:
            if isinstance(value, dict) and isinstance(target[key], dict):
                # Check for explicit override
                if value.get("merge_behavior") == "override_existing":
                    target[key] = value
                else:
                    # Merge children arrays
                    if "children" in value and "children" in target[key]:
                        target[key]["children"].extend(value["children"])
                    # Merge other dict properties recursively
                    merge_menu_data(target[key], value, current_path)
            elif key == "children" and isinstance(value, list) and isinstance(target[key], list):
                # Directly extend children arrays
                target[key].extend(value)
            elif value.get("merge_behavior") == "override_existing":
                # Allow explicit overrides
                target[key] = value
            else:
                # Only raise for conflicts in non-children fields
                if key != "children":
                    raise ValueError(f"Menu item conflict at '{current_path}' - explicit override required")
        else:
            # New item, simply add it
            target[key] = value


def parse_menu_item(data: Dict, action_names: Set[str], enabled_features: Set[str]) -> MenuItem:
    """Parse single menu item and collect action names"""
    if not should_include_item(data, enabled_features):
        return None

    missing_fields = []
    if "label" not in data:
        missing_fields.append("label")
    if "type" not in data:
        missing_fields.append("type")

    if missing_fields:
        raise ValueError(f"Menu item missing required fields: {', '.join(missing_fields)}\nData: {data}")


    # Handle conditions.feature_enabled shorthand
    conditions = None
    if "conditions" in data:
        conditions = parse_conditions(data["conditions"])

    # Parse operation if present and collect action name
    operation = None
    if "operation" in data:
        operation = parse_operation(data["operation"])
        if operation.action:
            action_names.add(operation.action)

    # Parse children recursively
    children = []
    if "children" in data:
        children = [child for child in (parse_menu_item(child, action_names, enabled_features) for child in data["children"]) if child is not None]

    # Parse children recursively and sort by label
    children = []
    if "children" in data:
        child_data = sorted(data["children"], key=lambda x: x["label"])
        children = [child for child in (parse_menu_item(child, action_names, enabled_features)
                   for child in child_data) if child is not None]

    return MenuItem(
        label=data["label"],
        label_short=data.get("label_short"),
        icon=data.get("icon"),
        shortcut=data.get("shortcut"),
        type=data["type"],
        help_text=data.get("help_text"),
        operation=operation,
        conditions=conditions,
        children=children,
        enabled_by=data.get("enabled_by")
    )


def parse_operation(data: Dict[str, Any]) -> Operation:
    """Parse operation configuration"""
    precondition_config = None
    if "precondition" in data:
        precondition_config = PreconditionConfig(
            handler=data["precondition"]["handler"],
            message=data["precondition"]["message"],
            args=data["precondition"].get("args")
        )

    input_configs = None
    if "input" in data:
        input_configs = [parse_input_config(input_data)
                        for input_data in data["input"]]

    confirm_config = None
    if "confirm" in data:
        confirm_config = parse_confirm_config(data["confirm"])

    result_config = None
    if "result" in data:
        result_config = parse_result_config(data["result"])

    postcondition_config = None
    if "postcondition" in data:
        postcondition_config = PostconditionConfig(
            handler=data["postcondition"]["handler"],
            message=data["postcondition"]["message"],
            args=data["postcondition"].get("args")
        )

    return Operation(
        action=data["action"],
        precondition=precondition_config,
        inputs=input_configs,
        confirm=confirm_config,
        result=result_config,
        postcondition=postcondition_config
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
        if "value_equals" in rule_data:
            value_data = rule_data["value_equals"]
            rules.append(ValueRule(
                variable=value_data["variable"],
                value=value_data["value"]
            ))
        elif "value_compare" in rule_data:
            value_data = rule_data["value_compare"]
            rules.append(ValueCompareRule(
                variable=value_data["variable"],
                value=value_data["value"],
                operator=value_data["operator"]
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
