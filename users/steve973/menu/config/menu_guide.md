# Keyboard Menu JSON Structure and Definition

* [Overview](#overview)
* [Core Menu Item Properties](#core-menu-item-properties)
* [Menu Types](#menu-types)
* [Operation Lifecycle](#operation-lifecycle)
* [Input Types](#input-types)
* [Conditions and Features](#conditions-and-features)
* [Menu Options](#menu-options)
* [Variable Substitution](#variable-substitution)
* [Examples](#examples)
* [Implementation Notes](#implementation-notes)

## Overview

### Purpose
The QMK JSON Menu system provides a declarative way to define OLED-based menus
for keyboard configuration, control, and status. By separating menu structure
and behavior from its implementation, it enables flexible and maintainable menu
creation that requires only minimal addition of c code, and it reduces nearly
all of the builderplate code associated with common menu patterns and tasks.
Many of the basic and standard functions of a menu system are built in, including
input range or choices, confirmation dialogs, and notification/result of an
action's completion are configuration items, so you can specify these things in
JSON and they will be applied to your menu. When you need a capability that
goes beyond these things, you will be able to customize the behavior to suit
your needs.

### Key Benefits
- **Declarative Configuration**: Define complete menu structures in JSON
- **Flexible Operations**: Multi-phase operations with input, confirmation and results
- **Feature Awareness**: Conditional menu items based on enabled features
- **Consistent UX**: Standardized navigation and interaction patterns
- **Extensible**: Support for custom handlers and variable substitution
- **Self-Documenting**: Integrated help text and documentation

### Basic Structure
```json
{
    "main_menu": {
        "label": "Main Menu",
        "type": "submenu", 
        "children": [
            {
                "label": "Example Item",
                "shortcut": "KC_E",
                "type": "action",
                "help_text": "Example menu item description",
                "operation": {
                    "action": "example_function",
                    "input": [{
                        "type": "options",
                        "options": ["A", "B", "C"]
                    }],
                    "result": {
                        "message": "Selected: {value}",
                        "mode": "timed",
                        "timeout_sec": 1
                    }
                }
            }
        ]
    }
}
```
The menu system starts with a root menu containing child items. Each item defines
its display properties, behavior, and optional operation details like input
handling and result display.

## Core Menu Item Properties

Core menu items have both required, and optional, properties in order to provide
the features of a useful menu system.

### Required Properties
- `label` (string): Display text shown in the menu
- `type` (string): Type of menu item - must be one of:
  - `"action"` - Executes a function
  - `"submenu"` - Contains child menu items  
  - `"display"` - Shows information only

### Optional Properties
- `shortcut` (string): Keyboard shortcut in QMK keycode format (e.g. "KC_S")
- `help_text` (string): Description shown when help is requested
- `children` (array): Required for submenu types, contains child menu items
- `operation` (object): Required for action types, defines behavior
- `conditions` (object): Controls when item is shown/enabled

### Property Types and Values

#### Type-Specific Properties
```json
// Action item
{
    "label": "Reset EEPROM",
    "type": "action",
    "operation": {
        "action": "reset_eeprom"
    }
}

// Submenu item 
{
    "label": "RGB Settings",
    "type": "submenu",
    "children": []
}

// Display item
{
    "label": "System Info",
    "type": "display",
    "operation": {
        "action": "show_system_info"
    }
}
```

#### Operation Object
When `type` is "action", the operation object can contain:
```json
"operation": {
    "action": "function_name",      // Required - function to call
    "input": [{ }],                 // Optional input configuration
    "confirm": { },                 // Optional confirmation dialog
    "result": { }                   // Optional result display
}
```

#### Conditions Object
These are optional conditions that control item visibility/availability:
```json
"conditions": {
    "feature_enabled": "RGB_MATRIX_ENABLE",  // Only show if feature enabled
    "value_equals": {                        // Show based on value
        "variable": "keyboard_mode",
        "value": "gaming"
    }
}
```

## Menu Types

### Action Items
These are enu items that execute functions when selected. They can include input
gathering, confirmation dialogs, and result display.

```json
{
    "label": "Reset EEPROM",
    "type": "action",
    "help_text": "Reset all settings to defaults",
    "operation": {
        "action": "reset_eeprom",
        "confirm": {
            "message": "Reset all settings?"
        }
    }
}
```

### Submenus 
These are "container" items that hold other menu items. They are used to create
hierarchical menu structures. I.e., this is how you define submenus.

```json
{
    "label": "RGB Settings",
    "type": "submenu",
    "help_text": "Configure RGB matrix settings",
    "children": [
        {
            "label": "Toggle RGB",
            "type": "action",
            "operation": {
                "action": "toggle_rgb"
            }
        }
    ]
}
```

### Display Items
These are menu items that show information without taking action. They are useful
for status displays and information pages.

```json
{
    "label": "System Info",
    "type": "display",
    "help_text": "View system configuration",
    "operation": {
        "action": "show_system_info"
    }
}
```

## Operation Lifecycle

Menu operations can have multiple phases that execute in sequence. This sequence
consists of the following phases:

- `precondition`: Must evaluate to true in order to proceed
- `input`: Data from the user that is needed to perform the action
- `confirmation`: Explicit authorization from the user to proceed
- `action`: Execute the task
- `result`: Display the outcome of the task to the user
- `postcondition`: Verification/cleanup/etc after the task has been completed

Each phase is optional except for the action phase. It is also important to know
that *all* operation lifecycle phases will run, regardless of a failure during
any of the phases.  The operation lifecycle phases all return a result of their
execution, and that result is passed to the next phase.  This ensures a
consistent operation lifecycle workflow, and any subsequent phase can respond
to a previous phase's result.  For example, the `postcondition` phase can
perform a cleanup, or revert any actions that occurred prior to, or during, a
failure in an earlier phase.

### Precondition Phase
Optional checks or setup before starting an operation.

```json
"precondition": {
    "handler": "check_joystick_ready",
    "message": "Checking joystick state...",
    "args": {
        "timeout_ms": 5000
    }
}
```

### Input Phase
Gather user input through various methods. Input can be a single value or an
array of inputs that will be collected in sequence.

Single input example:

```json
"input": [{
    "type": "range",
    "range": "0..100:5",
    "prompt": "Set sensitivity:",
    "default": "{current_value}",
    "wrap": true,
    "live_preview": true
}]
```

Multiple input example:

```json
"input": [
    {
        "type": "range",
        "range": "0..360:1",
        "prompt": "Set Hue:",
        "default": "{current_hue}",
        "wrap": true
    },
    {
        "type": "range",
        "range": "0..100:1",
        "prompt": "Set Saturation:",
        "default": "{current_sat}"
    },
    {
        "type": "range",
        "range": "0..100:1",
        "prompt": "Set Value:",
        "default": "{current_val}"
    }
]
```

Mixed input types example:

```json
"input": [
    {
        "type": "options",
        "options": ["Static", "Breathing", "Rainbow"],
        "prompt": "Select effect:",
        "default": "{current_effect}"
    },
    {
        "type": "range",
        "range": "0..255:8",
        "prompt": "Set speed:",
        "default": "{current_speed}"
    }
]
```

For range inputs, the format is `min_value`..`max_value`:`step`

Inputs are collected in sequence and passed to the action handler as an array
with the same number of elements, and in the same order that they are defined.
Each input's value can be referenced in subsequent prompts or in
confirmation/result messages using `{values[0]}`, `{values[1]}`, etc.

### Confirmation Phase
Request user confirmation before proceeding.

```json
"confirm": {
    "message": "Reset all settings?\nThis cannot be undone!",
    "timeout_sec": 30,
    "default": false,
    "true_text": "Reset",
    "false_text": "Cancel"
}
```

### Action Phase
Required - executes the main operation function.

```json
"action": "reset_eeprom"  // Function name to call
```

### Result Phase
Display operation result or status.

```json
"result": {
    "message": "Settings reset complete\nRebooting...",
    "mode": "timed",      // or "acknowledge"
    "timeout_sec": 2,
    "ok_text": "Continue" // For acknowledge mode
}
```

### Postcondition Phase
Optional verification or cleanup after operation.

```json
"postcondition": {
    "handler": "verify_settings_reset",
    "message": "Verifying reset...",
    "args": {
        "retries": 3
    }
}
```

### Complete Example
```json
{
    "label": "Reset Settings",
    "type": "action",
    "operation": {
        "precondition": {
            "handler": "check_system_ready",
            "message": "Checking system state..."
        },
        "input": [{
            "type": "options",
            "options": ["All", "RGB Only", "Keymap Only"],
            "prompt": "Select reset type:"
        }],
        "confirm": {
            "message": "Reset {value}?\nThis cannot be undone!",
            "timeout_sec": 30,
            "default": false
        },
        "action": "reset_settings",
        "result": {
            "message": "Reset complete: {status}",
            "mode": "acknowledge"
        },
        "postcondition": {
            "handler": "verify_reset",
            "message": "Verifying reset..."
        }
    }
}
```

### Operation Handlers

Each phase in the operation lifecycle is handled by a function that follows a
consistent signature:

```c
operation_result_t handler(operation_result_t prev_result, void** input_values);
```

The handler parameters are:
- `prev_result`: Result from the previous phase's execution
- `input_values`: Array of collected input values (primarily used by action phase)

Each handler can:
- Examine the previous phase's result to decide how to proceed
- Access any collected input values (in action phase)
- Return its own result that will be passed to the next phase

Example handler:

```c
operation_result_t set_rgb_handler(operation_result_t prev_result, void** input_values) {
    // Check previous phase result
    if (prev_result != OPERATION_RESULT_SUCCESS) {
        return prev_result;  // Propagate failure
    }
    
    // Access input values in order they were collected
    uint8_t effect = (uint8_t)input_values[0];
    uint8_t speed = (uint8_t)input_values[1];
    
    // Perform the action
    bool success = rgb_matrix_set_effect(effect, speed);
    
    return success ? OPERATION_RESULT_SUCCESS : OPERATION_RESULT_ERROR;
}
```

The operation result can be one of:
- `OPERATION_RESULT_SUCCESS`: Phase completed successfully
- `OPERATION_RESULT_CANCELLED`: User cancelled the operation
- `OPERATION_RESULT_REJECTED`: Operation was rejected (e.g., failed validation)
- `OPERATION_RESULT_ERROR`: An error occurred
- `OPERATION_RESULT_TIMEOUT`: Operation timed out

Each phase in the lifecycle (precondition, input collection, confirmation, action,
result display, postcondition) uses this same handler signature, providing a
consistent interface throughout the operation's execution.

## Input Types

Menu operations can gather user input in several ways. Each input type has specific
configuration options. When multiple inputs are needed, they can be combined in
an array and will be collected in sequence.

### Range Input
For numeric value selection within a defined range.

Single value example:

```json
"input": [{
    "type": "range",
    "range": "0..255:8",     // min..max:step
    "prompt": "Brightness:",
    "default": 128,
    "wrap": true,            // Wrap around at min/max
    "live_preview": true     // Update as value changes
}]
```

Multiple related values example:

```json
"input": [
    {
        "type": "range",
        "range": "0..255:1",
        "prompt": "Red:",
        "default": "{current_red}",
        "wrap": true,
        "live_preview": true
    },
    {
        "type": "range",
        "range": "0..255:1",
        "prompt": "Green:",
        "default": "{current_green}",
        "wrap": true,
        "live_preview": true
    },
    {
        "type": "range",
        "range": "0..255:1",
        "prompt": "Blue:",
        "default": "{current_blue}",
        "wrap": true,
        "live_preview": true
    }
]
```

### Options Input
For selecting from a list of predefined choices.

Single selection example:

```json
"input": [{
    "type": "options",
    "options": ["Arrows", "WASD", "Analog", "Mouse"],
    "options.conditions": {          // Optional conditions per option
        "Mouse": "POINTING_DEVICE_ENABLE"
    },
    "prompt": "Select mode:",
    "default": "{current_mode}",
    "wrap": true,                    // Wrap around at ends
    "display_values": "{mode_names}" // Optional friendly names
}]
```

Combined with other input types:

```json
"input": [
    {
        "type": "options",
        "options": ["Wave", "Reactive", "Breathing"],
        "prompt": "Effect type:",
        "default": "{current_effect}"
    },
    {
        "type": "range",
        "range": "0..10:1",
        "prompt": "Effect speed:",
        "default": "{current_speed}",
        "live_preview": true
    }
]
```

### Custom Input Handler
For complex input that requires custom logic. This can be useful, for example,
when you have to wait for a condition to become true, or if you need to
instruct the user to do something, and you need to verify its completion
before proceeding. Since this calls a method, the only requirement is that
the method returns true or false to indicate if the action can proceed, or if
it should be aborted.

    ```json
    "input": [{
        "type": "custom",
        "handler": "wait_for_js_movement",
        "args": {
            "timeout_ms": 10000,
            "threshold": 85
        },
        "prompt": "Move joystick in circles",
        "cancellable": true
    }]
    ```

Multiple custom inputs example:
    ```json
    "input": [
        {
            "type": "custom",
            "handler": "calibrate_center",
            "prompt": "Center the joystick and press Enter"
        },
        {
            "type": "custom",
            "handler": "calibrate_edges",
            "prompt": "Move joystick in circles to calibrate edges"
        }
    ]
    ```

### Common Input Properties
Properties available for all input types:
- `prompt` (string): Text shown during input
- `default` (any): Initial value
- `wrap` (boolean): Whether selection wraps at bounds
- `live_preview` (boolean): Update as value changes

Input values are collected in sequence and passed to the action handler in the
same order they appear in the input array. Values can be referenced in
confirmation or result messages using array syntax: `{values[0]}`, `{values[1]}`,
etc.

When designing multi-input operations:
- Order inputs logically for the user
- Group related inputs together
- Consider using live_preview when appropriate
- Keep the number of inputs reasonable
- Use clear, concise prompts
- Provide meaningful defaults when possible

## Conditions and Features

Menu items and options can be conditionally displayed or enabled based on various
conditions.

### Feature Gating
Control menu item visibility based on enabled QMK features.

```json
{
    "label": "RGB Settings",
    "type": "submenu",
    "conditions": {
        "match": "all",
        "rules": [
            { "feature_enabled": "RGB_MATRIX_ENABLE" }
        ]
    },
    "children": []
}
```

### Option Conditions
Control availability of specific options within option inputs.

```json
"input": [{
    "type": "options",
    "options": ["Arrows", "WASD", "Analog", "Mouse"],
    "options.conditions": {
        "Mouse": {
            "match": "all",
            "rules": [
                { "feature_enabled": "POINTING_DEVICE_ENABLE" }
            ]
        }
    }
}]
```

### Value Conditions
Show or hide items based on system values or states.

```json
"conditions": {
    "match": "all",
    "rules": [
        {
            "value_equals": {
                "variable": "keyboard_mode",
                "value": "gaming"
            }
        },
        {
            "value_compare": {
                "variable": "rgb_matrix_config.power_draw_mA",
                "operator": "less_than",
                "value": 500
            }
        }
    ]
}
```

### Multiple Conditions
Specify whether all conditions must be met (`all`) or any condition can be met
(`any`).

```json
"conditions": {
    "match": "all",  // or "any"
    "rules": [
        {
            "feature_enabled": "RGB_MATRIX_ENABLE"
        },
        {
            "value_equals": {
                "variable": "rgb_matrix_config.mode",
                "value": "RGB_MATRIX_SOLID_COLOR"
            }
        }
    ]
}
```

For complex logic, conditions can be nested:

```json
"conditions": {
    "match": "any",
    "rules": [
        {
            "match": "all",
            "rules": [
                { "feature_enabled": "RGB_MATRIX_ENABLE" },
                { 
                    "value_equals": {
                        "variable": "rgb_matrix_config.mode",
                        "value": "RGB_MATRIX_SOLID_COLOR"
                    }
                }
            ]
        },
        {
            "feature_enabled": "LED_MATRIX_ENABLE"
        }
    ]
}
```

### Available Operators
For value comparisons:
- `equals`
- `not_equals`
- `greater_than`
- `less_than`
- `greater_equals`
- `less_equals`

Variables can reference any system value that the menu implementation exposes
for condition checking.

## Menu Options

The menu system's behavior can be configured in several ways:

### Display Configuration
- Show/hide keyboard shortcuts next to menu items (e.g. "Reset EEPROM (R)")
- Choose selection indicator style (highlight, arrow, or star)
- Set default timeouts for results and confirmations
- Configure menu timeout with visual indicator

### Navigation Settings
- Arrow keys always enabled for menu navigation
- Optional WASD key navigation
- Configurable shortcut behavior:
  - Direct mode: Shortcut immediately executes action
  - Select mode: First press selects item, second press executes
- Escape key always returns to previous menu
- Enter key selects/executes current item

### Timeout Behavior  
- Configurable menu inactivity timeout (5-120 seconds)
- Visual countdown indicator on right edge of display
- Auto-exit to normal keyboard operation when timeout reached
- Configurable timeouts for:
  - Result messages (1-10 seconds)
  - Confirmation dialogs (10-60 seconds)
  - Input prompts

### Default Values
- Menu timeout: 30 seconds
- Result display: 2 seconds
- Confirmation timeout: 30 seconds
- Selection style: Highlight
- Shortcuts: Hidden
- WASD navigation: Disabled
- Shortcut mode: Direct execute

## Variable Substitution

The menu system supports variable substitution in messages, prompts, and default
values. Variables are referenced using curly braces.

Variables can be used in:
- Help text
- Prompts
- Messages
- Default values
- Condition checks

### System Variables
These are system-wide values from QMK, and they are available throughout the
menu system.

```json
{
    "label": "Memory Usage",
    "type": "display",
    "help_text": "Current memory: {memory_used}/{memory_total}KB\nFree: {memory_free}KB"
}
```

Common system variables:
- `{firmware_version}`
- `{keyboard_name}`
- `{manufacturer}`
- `{uptime}`
- `{memory_used}`
- `{memory_free}`

### Feature Values
These provide access to the current state of QMK features.

```json
{
    "label": "RGB Brightness",
    "operation": {
        "input": [{
            "type": "range",
            "range": "0..255:16",
            "default": "{rgb_matrix_val}",
            "prompt": "Current: {rgb_matrix_val}",
            "wrap": true,
            "live_preview": true
        }],
        "result": {
            "message": "Brightness set to {value}\nPower draw: {rgb_matrix_power_draw}mA"
        }
    }
}
```

Common feature variables:
- `{rgb_matrix_mode}`
- `{rgb_matrix_val}`
- `{oled_brightness}`
- `{audio_enable}`

### Current States
Several menu-specific variables and values are available during menu operations.
These variables are provided by different parts of the implementation:

#### Menu System Variables
Provided by the core menu implementation:
- `{value}` - Current input value being processed
- `{previous_value}` - Previous value before change
- `{selected_index}` - Current menu selection (0-based)
- `{menu_timeout}` - Remaining timeout seconds

#### Implementation-Specific Variables
In order to provide information related specifically to the action invoked by
a menu entry, such as the current value of the item being affected, the menu
system maps variable names to getter functions:

```c
// Variable mapping in menu implementation
struct variable_mapping {
    const char* name;           // Variable name used in JSON
    getter_func_t get_value;    // Function pointer to get value
} variable_mappings[] = {
    // Joystick mappings
    {"current_stick_mode", get_current_stick_mode},
    {"joystick_sensitivity", get_joystick_sensitivity},
    
    // RGB matrix mappings  
    {"rgb_matrix_mode", get_rgb_matrix_mode},
    {"rgb_matrix_val", get_rgb_matrix_val}
};
```

When the menu system encounters a variable like `{current_stick_mode}`, it:
1. Looks up the variable name in mappings
2. Calls the associated getter function
3. Converts returned value to string
4. Substitutes result in text

Example:
```json
{
    "label": "Joystick Mode",
    "operation": {
        "input": [{
            "default": "{current_stick_mode}",  // Menu system calls get_current_stick_mode()
            "prompt": "Current: {current_stick_mode}" 
        }]
    }
}
```

Menu features register their variables by adding entries to the mapping table.

## Examples
- Basic Menu Items
- Complex Operations
- Conditional Features
- Custom Input Handling

## Implementation Notes
- Memory Considerations
- Performance Tips 
- Best Practices
