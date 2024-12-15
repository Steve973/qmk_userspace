# Multi-Function Display (MFD) System User Guide

* [Overview](#overview)
* [Core Concepts](#core-concepts)
* [Screen Types](#screen-types)
* [Configuration](#configuration)
* [Display Positioning](#display-positioning)
* [Screen Management](#screen-management)
* [Value Pairs](#value-pairs)
* [Examples](#examples)
* [Best Practices](#best-practices)

## Overview

### Purpose
The QMK Multi-Function Display (MFD) system provides a flexible and maintainable
way to create OLED-based information displays for your keyboard. It supports
both simple text/value displays and custom rendering functions, with automatic
screen cycling, timeout management, and positioning controls.

### Key Benefits
- **Simple Configuration**: Define display screens through straightforward
        structures
- **Flexible Layouts**: Automatic or manual positioning of content
- **Dynamic Updates**: Configurable refresh intervals per screen
- **Screen Management**: Built-in screen cycling and timeout handling
- **Extensible**: Support for both simple displays and custom rendering
- **Memory Efficient**: Reusable structures for common display patterns

### Basic Structure
```c
// Define value pairs
static mfd_value_pair_t status_pairs[] = {
    { .label = "WPM",    .get_value = get_wpm_value },
    { .label = "Layer",  .get_value = get_layer_value }
};

// Define screens
mfd_screen_t mfd_screens[] = {
    {
        .title = "Status Display",
        .type = MFD_TYPE_SIMPLE,
        .refresh_interval = 1000,
        .position_mode = MFD_POSITION_AUTO,
        .auto_align = MFD_AUTO_ALIGN_TOP,
        .display.simple = {
            .pairs = status_pairs,
            .pair_count = sizeof(status_pairs) / sizeof(status_pairs[0])
        }
    }
};

// Configure MFD system
mfd_config_t mfd_config = {
    .screens = mfd_screens,
    .screen_count = sizeof(mfd_screens) / sizeof(mfd_screens[0]),
    .default_index = 0,
    .timeout_ms = 30000,
    .cycle_screens = true
};
```

## Core Concepts

### Screen Types
The MFD system supports two primary types of screens:

1. **Simple Screens** (MFD_TYPE_SIMPLE)
   - Display label/value pairs
   - Automatic or manual positioning
   - Configurable refresh intervals
   - Built-in formatting and layout

2. **Custom Screens** (MFD_TYPE_CUSTOM)
   - Full control over rendering
   - Custom refresh handling
   - Direct OLED access

### Screen Configuration
Each screen requires certain properties:
```c
typedef struct {
    const char* title;                  // Screen title
    mfd_screen_type_t type;            // Simple or Custom
    uint16_t refresh_interval;          // In milliseconds
    mfd_position_mode_t position_mode;  // Auto or Manual
    mfd_auto_align_t auto_align;        // Top or Center alignment
    
    union {
        struct {
            mfd_value_pair_t* pairs;    // Array of value pairs
            uint8_t pair_count;         // Number of pairs
            uint8_t start_row;          // For manual positioning
            uint8_t start_col;          // For manual positioning
        } simple;

        struct {
            void (*render)(void);       // Custom render function
        } custom;
    } display;
} mfd_screen_t;
```

## Screen Types

### Simple Screens
For displaying formatted label/value pairs:

```c
// Define value getters
static const char* get_wpm_value(void) {
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", get_current_wpm());
    return buffer;
}

// Define value pairs
static mfd_value_pair_t status_pairs[] = {
    { .label = "WPM", .get_value = get_wpm_value }
};

// Create simple screen
mfd_screen_t wpm_screen = {
    .title = "Speed",
    .type = MFD_TYPE_SIMPLE,
    .refresh_interval = 1000,
    .position_mode = MFD_POSITION_AUTO,
    .auto_align = MFD_AUTO_ALIGN_TOP,
    .display.simple = {
        .pairs = status_pairs,
        .pair_count = 1
    }
};
```

### Custom Screens
For complete control over rendering:

```c
static void render_logo(void) {
    // Custom OLED rendering code
    oled_write_P(PSTR("Custom\nLogo"), false);
}

mfd_screen_t logo_screen = {
    .type = MFD_TYPE_CUSTOM,
    .refresh_interval = 0,  // Static display
    .display.custom = {
        .render = render_logo
    }
};
```

## Configuration

### System Configuration
The MFD system is configured through the mfd_config structure:

```c
mfd_config_t mfd_config = {
    .screens = mfd_screens,          // Array of screen definitions
    .screen_count = SCREEN_COUNT,    // Number of screens
    .default_index = 0,              // Default screen to show
    .timeout_ms = 30000,             // Return to default after 30s
    .cycle_screens = true            // Auto-cycle through screens
};
```

### Screen Configuration
Individual screen configuration options:

```c
mfd_screen_t screen = {
    .title = "Status",              // Screen title
    .type = MFD_TYPE_SIMPLE,        // Screen type
    .refresh_interval = 1000,       // Update every 1000ms
    .position_mode = MFD_POSITION_AUTO,
    .auto_align = MFD_AUTO_ALIGN_TOP
};
```

## Display Positioning

### Automatic Positioning
Let the framework handle layout:

```c
mfd_screen_t auto_screen = {
    .position_mode = MFD_POSITION_AUTO,
    .auto_align = MFD_AUTO_ALIGN_TOP,  // or CENTER
    // ... other config
};
```

### Manual Positioning
Specify exact positions:

```c
mfd_screen_t manual_screen = {
    .position_mode = MFD_POSITION_MANUAL,
    .display.simple = {
        .start_row = 2,    // Start at row 2
        .start_col = 1     // Indent 1 column
    }
    // ... other config
};
```

## Screen Management

### Screen Switching
Switch between screens:

```c
// Switch to specific screen
void switch_to_screen(uint8_t index) {
    mfd_switch_screen(index);
}

// Return to default screen
void show_default(void) {
    mfd_switch_screen(mfd_config.default_index);
}
```

### Screen Cycling
Enable automatic screen cycling:

```c
mfd_config_t config = {
    .cycle_screens = true,     // Enable cycling
    .timeout_ms = 5000,        // Switch every 5 seconds
    // ... other config
};
```

## Value Pairs

### Creating Value Pairs
Define pairs of labels and value-getting functions:

```c
// Value getter function
static const char* get_layer_name(void) {
    static char buffer[16];
    uint8_t layer = get_highest_layer(layer_state);
    snprintf(buffer, sizeof(buffer), "%s", layer_name(layer));
    return buffer;
}

// Value pair definition
static mfd_value_pair_t layer_pair = {
    .label = "Layer",
    .get_value = get_layer_name
};
```

### Using Value Pairs
Group related pairs into screens:

```c
static mfd_value_pair_t keyboard_status[] = {
    { .label = "Layer",  .get_value = get_layer_name },
    { .label = "WPM",    .get_value = get_wpm_value },
    { .label = "Mods",   .get_value = get_mods_status }
};

mfd_screen_t status_screen = {
    .title = "KB Status",
    .type = MFD_TYPE_SIMPLE,
    .display.simple = {
        .pairs = keyboard_status,
        .pair_count = sizeof(keyboard_status) / sizeof(keyboard_status[0])
    }
    // ... other config
};
```

## Examples

### Basic Status Display
```c
// Value getters
static const char* get_wpm(void) {
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", get_current_wpm());
    return buffer;
}

static const char* get_layer(void) {
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%s", layer_name(get_highest_layer(layer_state)));
    return buffer;
}

// Value pairs
static mfd_value_pair_t status_pairs[] = {
    { .label = "WPM",   .get_value = get_wpm },
    { .label = "Layer", .get_value = get_layer }
};

// Screen definition
static mfd_screen_t status_screen = {
    .title = "Status",
    .type = MFD_TYPE_SIMPLE,
    .refresh_interval = 1000,
    .position_mode = MFD_POSITION_AUTO,
    .auto_align = MFD_AUTO_ALIGN_TOP,
    .display.simple = {
        .pairs = status_pairs,
        .pair_count = sizeof(status_pairs) / sizeof(status_pairs[0])
    }
};
```

### Custom Rendering Screen
```c
// Custom render function
static void render_logo(void) {
    static const char PROGMEM logo[] = {
        // Logo data
    };
    oled_write_raw_P(logo, sizeof(logo));
}

// Screen definition
static mfd_screen_t logo_screen = {
    .title = "Logo",
    .type = MFD_TYPE_CUSTOM,
    .refresh_interval = 0,
    .display.custom = {
        .render = render_logo
    }
};
```

### Multiple Screen Configuration
```c
// Define screens
mfd_screen_t screens[] = {
    // Status screen
    {
        .title = "Status",
        .type = MFD_TYPE_SIMPLE,
        .refresh_interval = 1000,
        .display.simple = {
            .pairs = status_pairs,
            .pair_count = STATUS_PAIR_COUNT
        }
    },
    // Logo screen
    {
        .title = "Logo",
        .type = MFD_TYPE_CUSTOM,
        .display.custom = {
            .render = render_logo
        }
    }
};

// Configure MFD
mfd_config_t mfd_config = {
    .screens = screens,
    .screen_count = sizeof(screens) / sizeof(screens[0]),
    .default_index = 0,
    .timeout_ms = 30000,
    .cycle_screens = true
};
```

## Best Practices

### Memory Management
1. Use static const for fixed strings
2. Keep value buffers reasonably sized
3. Reuse value pair arrays where possible
4. Use PROGMEM for large custom graphics

### Performance
1. Set appropriate refresh intervals
2. Minimize string operations in value getters
3. Cache values that are expensive to compute
4. Use static buffers in value getter functions

### Organization
1. Group related value pairs together
2. Use meaningful screen titles
3. Order screens logically
4. Keep value getter functions close to their pairs

### Error Handling
1. Always bounds-check array indexes
2. Provide reasonable defaults
3. Handle NULL returns from value getters
4. Validate configuration values

### Display Layout
1. Keep information density appropriate
2. Use consistent spacing
3. Group related information
4. Consider screen readability
