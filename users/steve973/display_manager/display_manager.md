# Display Manager Guide

## Overview

The Display Manager provides an abstraction layer for managing display content
on QMK keyboards, supporting both OLED and Quantum Painter (QP) displays. It
simplifies the process of creating and managing screen content through a unified
interface, regardless of the underlying display technology.

This is a lightweight abstraction, so it does not provide the full functionality
across both the OLED driver and the Quantum Painter APIs.  It does, however,
provide most of the features that are common across both APIs.  The user will
need to use these APIs directly if there are needed capabilities that this
abstraction does not provide.

## Key Features

- Unified interface for OLED and QP displays
- Screen stack management
- Automatic display refresh handling
- Support for different content types
- Centered content positioning
- Consistent text rendering with optional underlining and highlighting

## Basic Concepts

### Content Coordinates

The Display Manager handles coordinate systems differently depending on the
underlying display technology being used:

#### OLED Driver
When using the OLED driver, coordinates are character-based. The display is
divided into a grid where:
- Each character occupies a 6x8 pixel space
- X coordinates represent character columns (0 = leftmost)
- Y coordinates represent character rows (0 = topmost)
- For a 128x128 display, this results in approximately 21x16 characters

Example OLED positioning:
```c
screen_element_t element = {
    .x = 0,  // First character column
    .y = 1   // Second character row
};
```

#### Quantum Painter
With Quantum Painter, coordinates are pixel-based, providing finer control:
- X coordinates range from 0 to display width - 1
- Y coordinates range from 0 to display height - 1
- Text positioning accounts for font metrics automatically
- Images can be positioned at any pixel coordinate

Example QP positioning:
```c
screen_element_t element = {
    .x = 64,  // Center of 128px wide display
    .y = 32   // About 1/4 down a 128px high display
};
```

#### Abstraction Handling
The Display Manager internally handles the translation between these coordinate
systems. When you specify coordinates in your screen content:
- For OLED: Values are used directly as character positions
- For QP: Values are scaled appropriately based on font metrics

For most use cases, you can use the same coordinate values regardless of the
display technology, and the Display Manager will handle the appropriate
positioning. The main consideration is ensuring your content fits within the
display's viewable area.

### Screen Content

A screen consists of various elements that can be:
- Text items
- Key-value pairs
- Lists with optional highlighting
- Images

Each element has positioning information and content-specific properties.

### Example: Creating a Basic Screen

The process of creating a basic screen is straightforward, and requires four
main steps:

1. Create a screen content pointer (`screen_content_t`) by allocating memory.
2. Create a screen elements array (`screen_element_t`) by allocating memory for
   each of the items that you will be displaying.
3. Populate the elements with things like a screen title and list or key-value
   elements.
4. Aggregate all of these into screen content by creating a `screen_content_t`
   instance that will be the actual rendered content in a managed screen.

```c
screen_content_t* create_status_screen(void) {
    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) return NULL;
    
    screen_element_t* elements = malloc(sizeof(screen_element_t) * 2);
    if (!elements) {
        free(screen);
        return NULL;
    }

    // Create a title element
    elements[0] = (screen_element_t){
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Status",
            .value = {
                .static_value = "Active"
            },
            .is_dynamic = false
        }
    };

    // Create a list item
    elements[1] = (screen_element_t){
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text = {
                .static_text = "Press Enter"
            },
            .is_dynamic = false,
            .highlight_type = HIGHLIGHT_NONE
        }
    };

    // Now create the displayable screen content
    // with the elements created above
    *screen = (screen_content_t){
        .title = "System Status",
        .elements = elements,
        .element_count = 2,
        .center_contents = true
    };

    return screen;
}
```

## Screen Management

### Pushing a Screen

Once you hae defined a `screen_content_t` instance, you can use it to pupulate a
`managed_screen_t` that is the structure that is used by the display manager to
render this abstracted content to your disiplay via the OLED driver or the
Quantum Painter.  A managed screen requires several pieces of information:

1. The owner of the content.
2. Whether this managed screen requires custom rendering, or not.
3. The refresh interval for screen updates, which is particularly useful for
   screen content that is dynamic.

```c
void show_status_screen(void) {
    screen_content_t* content = create_status_screen();
    if (!content) return;

    push_screen((managed_screen_t){
        .owner = "STATUS",
        .is_custom = false,
        .display.content = content,
        .refresh_interval_ms = 1000  // Update every second
    });
}
```

### Popping a Screen

Only the owner of a screen can remove, or "pop" managed screen instances from
the screen stack.  The aforementioned owner needs to be supplied to the
`pop_screen` function in order for that screen to be popped from the screen
stack.

```c
void hide_status_screen(void) {
    pop_screen("STATUS");
}
```

## Content Types

Screen element content can contain a few types that are most commonly used in
QMK OLED displays.  These include:

1. Key-value pairs, where the values can be either static strings, or dynamic
   content that requires a function call to update the content that is rendered
   when the screen is refreshed.
2. List items, that are often used as menu entries, and they can be highlighted
   when a user selects them.  These are either static strings, or, like the
   dynamic text of key-value pairs, these can be updated via a function call
   immediately before screen refresh time.
3. Images that can be displayed by both the OLED driver or the Quantum Painter.
   Both implementations render the bytes in their respective formats.

### Key-Value Pairs

```c
screen_element_t element = {
    .type = CONTENT_TYPE_KEY_VALUE,
    .content.key_value = {
        .label = "Temperature",
        .value.static_value = "25°C"
    }
};
```

### List Items

```c
screen_element_t element = {
    .type = CONTENT_TYPE_LIST_ITEM,
    .content.list_item = {
        .text.static_text = "Select Me",
        .highlight_type = HIGHLIGHT_INVERTED
    }
};
```

### Images

```c
image_t qmk_logo = {
    .data = (const uint8_t*)&qmk_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};

screen_element_t qmk_logo_elements[] = {
    {
        .type = CONTENT_TYPE_IMAGE,
        .x = 0,
        .y = 0,
        .content.image = &qmk_logo
    }
};
```

### Dynamic Content

Content can be static or dynamic. Dynamic content uses callback functions:

```c
const char* get_temperature(void) {
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d°C", read_temperature());
    return buffer;
}

screen_element_t element = {
    .type = CONTENT_TYPE_KEY_VALUE,
    .content.key_value = {
        .label = "Temp",
        .value.get_value = get_temperature,
        .is_dynamic = true
    }
};
```

## Formatting of Text Content

The Display Manager provides several text formatting options to create visually
appealing and well-organized screens.

### Text Decoration

Text can be rendered with underlines, which can be particularly useful for
titles:

```c
// Basic underlined text
render_underlined_text("Basic Title", x, y);

// Advanced underline control
render_underlined_text_adv("Custom Title", x, y,
    2,      // Gap between text and underline
    1,      // Underline thickness
    false   // Invert colors
);
```

### Content Positioning

#### Horizontal Centering
Individual text elements can be centered on the display:

```c
uint16_t x_pos = calculate_center_xpos("Centered Text");
render_underlined_text("Centered Text", x_pos, y);
```

#### Vertical Centering
Groups of elements (like menu items or key-value pairs) can be centered
vertically on the display. The calculation accounts for:
- Whether there's a title
- Number of elements to display
- Available display height

```c
// Center a group of 3 menu items with a title
uint16_t start_y = calculate_center_ypos(3, true);

// Create elements using calculated Y position
screen_element_t elements[] = {
    {
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = start_y,     // First item
        .content = { /* ... */ }
    },
    {
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = start_y + 1, // Second item
        .content = { /* ... */ }
    },
    {
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = start_y + 2, // Third item
        .content = { /* ... */ }
    }
};
```

### Screen Content Centering

The Display Manager provides content centering through the `center_contents` flag
and helper functions:

```c
screen_content_t screen = {
    .title = "My Screen",
    .elements = elements,
    .element_count = element_count,
    .center_contents = true    // Enables horizontal centering
};
```

When `center_contents` is true:
- Elements are centered horizontally during rendering
- Vertical positioning must still be managed explicitly

For vertical positioning of element groups, use:
```c
// Calculate starting Y position for vertical centering
// Parameters: number of elements, whether screen has title
uint16_t start_y = calculate_center_ypos(num_elements, has_title);
```

For individual element horizontal centering:
```c
// Calculate X position to center specific text
uint16_t x_pos = calculate_center_xpos("Centered Text");
```

### Text Highlighting

List items can be highlighted in different ways:

```c
screen_element_t menu_item = {
    .type = CONTENT_TYPE_LIST_ITEM,
    .content.list_item = {
        .text.static_text = "Menu Item",
        .highlight_type = HIGHLIGHT_INVERTED  // Inverted colors
    }
};

screen_element_t prefix_item = {
    .type = CONTENT_TYPE_LIST_ITEM,
    .content.list_item = {
        .text.static_text = "Prefixed Item",
        .highlight_type = HIGHLIGHT_PREFIX,   // Adds prefix character
        .highlight.prefix_char = '>'          // Custom prefix
    }
};
```

### Text Content Formatting Considerations and Best Practices

1. Screen Layout
   - Use consistent spacing between elements
   - Center important information
   - Use underlining for titles and headers
   - Group related information

2. Text Formatting
   - Keep text concise to fit display width
   - Use highlighting sparingly for emphasis
   - Consider readability when inverting colors
   - Maintain consistent style across screens

3. Dynamic Content
   - Account for varying content length in centering calculations
   - Ensure dynamic text won't overflow display bounds
   - Consider using ellipsis for truncated content

## Display Refresh

The display manager automatically handles refresh timing.  The proper loop to
coordinate the screen refresh depends on whether you are using the OLED driver,
or the Quantum Painter.  Note that it may be a good idea to use a timer to 
keep the display from trying to refresh too quickly.  The topmost element of
the screen stack is displayed by calling `show_current_screen()`.

Quantum Painter should use `housekeeping_task_user()`:

```c
void housekeeping_task_user(void) {
    #ifdef QUANTUM_PAINTER_ENABLE
        // Update display every 50ms
        static int32_t display_timer = 0;
        int32_t now = timer_read32();
        if (now - display_timer >= 50) {
            display_timer = now;
            show_current_screen();
        }
    #endif
}
```

The OLED driver should use `oled_task_user()`:

```c
bool oled_task_user(void) {
    #ifdef QUANTUM_PAINTER_ENABLE
        static int32_t display_timer = 0;
        int32_t now = timer_read32();
        // Update display every 50ms
        if (now - display_timer >= 50) {
            display_timer = now;
            show_current_screen();
        }
        return false;
    #endif
}
```

## Best Practices

1. Memory Management
   - Free screens when no longer needed
   - Check malloc returns for NULL
   - Use stack allocation for temporary content

2. Screen Design
   - Keep content concise and readable
   - Use consistent layouts
   - Consider display size limitations

3. Performance
   - Use appropriate refresh intervals
   - Minimize dynamic content updates
   - Clean up unused screens

4. Error Handling
   - Always check return values
   - Provide fallback content
   - Clean up on failure

## Implementation Notes

The Display Manager internally handles:
- Screen positioning and centering
- Content type-specific rendering
- Display technology differences
- Screen stack management
- Automatic refresh timing

## Example: Complete Screen Implementation

```c
void create_settings_screen(void) {
    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) return;

    screen_element_t* elements = malloc(sizeof(screen_element_t) * 3);
    if (!elements) {
        free(screen);
        return;
    }

    // Status element
    elements[0] = (screen_element_t){
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Status",
            .value.get_value = get_system_status,
            .is_dynamic = true
        }
    };

    // Menu items
    elements[1] = (screen_element_t){
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text.static_text = "Configure",
            .highlight_type = HIGHLIGHT_NONE
        }
    };

    elements[2] = (screen_element_t){
        .type = CONTENT_TYPE_LIST_ITEM,
        .x = 0,
        .y = 3,
        .content.list_item = {
            .text.static_text = "Exit",
            .highlight_type = HIGHLIGHT_NONE
        }
    };

    *screen = (screen_content_t){
        .title = "Settings",
        .elements = elements,
        .element_count = 3,
        .center_contents = true
    };

    push_screen((managed_screen_t){
        .owner = "SETTINGS",
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 1000
    });
}
```

## Troubleshooting

Common issues and solutions:

1. Screen Not Updating
   - Check refresh interval
   - Verify dynamic content callbacks
   - Ensure show_current_screen() is called

2. Memory Issues
   - Verify all mallocs are checked
   - Ensure screens are properly freed
   - Check stack usage

3. Display Artifacts
   - Verify content fits display
   - Check positioning calculations
   - Ensure proper clearing between updates

## Further Reading

- QMK Documentation
- OLED Driver Documentation
- Quantum Painter Documentation
