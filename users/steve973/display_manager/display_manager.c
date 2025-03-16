#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "quantum/logging/debug.h"
#include "display_manager.h"
#include "timer.h"

#define IMG_ORIGIN 0, 0
#define DISPLAY_BUFFER_SIZE 22
#define BUTTON_SEPARATOR "    "

static char display_buffer[DISPLAY_BUFFER_SIZE];

/**
 * @brief A stack of managed screens.
 */
static screen_stack_t screen_stack = {
    .top = -1,
    .max_size = 10
};

/**
 * @brief The last time the screen was refreshed.
 */
static uint32_t last_refresh = 0;

/**
 * @brief Provides textual representations for screen_push_status_t entries.
 */
static const char* const screen_push_status_strings[] = {
    [SCREEN_PUSH_SUCCESS] = "SCREEN_PUSH_SUCCESS",
    [SCREEN_PUSH_FAIL_SCREEN_NULL] = "SCREEN_PUSH_FAIL_SCREEN_NULL",
    [SCREEN_PUSH_FAIL_OWNER_NULL] = "SCREEN_PUSH_FAIL_OWNER_NULL",
    [SCREEN_PUSH_FAIL_STACK_FULL] = "SCREEN_PUSH_FAIL_STACK_FULL",
    [SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK] = "SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK",
    [SCREEN_PUSH_FAIL_OWNER_MISMATCH] = "SCREEN_PUSH_FAIL_OWNER_MISMATCH"
};

/**
 * @brief Provides textual representations for screen_pop_status_t entries.
 */
static const char* const screen_pop_status_strings[] = {
    [SCREEN_POP_SUCCESS] = "SCREEN_POP_SUCCESS",
    [SCREEN_POP_FAIL_STACK_EMPTY] = "SCREEN_POP_FAIL_STACK_EMPTY",
    [SCREEN_POP_FAIL_OWNER_NULL] = "SCREEN_POP_FAIL_OWNER_NULL",
    [SCREEN_POP_FAIL_OWNER_MISMATCH] = "SCREEN_POP_FAIL_OWNER_MISMATCH",
    [SCREEN_POP_FAIL_SCREEN_NOT_IN_STACK] = "SCREEN_POP_FAIL_SCREEN_NOT_IN_STACK"
};

/**
 * @brief Calculate the starting x-position to center text on the display.
 *
 * @param text the element to calculate the center position for.
 * @return The x-coordinate to center the text.
 */
uint16_t calculate_center_position(const screen_element_t* element) {
    // Calculate string length based on element type
    switch(element->type) {
        case CONTENT_TYPE_KEY_VALUE: {
            const key_value_t* kv = &element->content.key_value;
            const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%s: %s", kv->label, value);
            break;
        }
        case CONTENT_TYPE_LIST_ITEM: {
            const list_item_t* item = &element->content.list_item;
            const char* text = item->is_dynamic ? item->text.get_text() : item->text.static_text;
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%s", text);
            break;
        }
        case CONTENT_TYPE_BUTTON:
            // The whole row would be centered, not the individual buttons.
        case CONTENT_TYPE_IMAGE:
        case CONTENT_TYPE_CUSTOM:
            break;
    }

    uint16_t x_pos = calculate_center_xpos(display_buffer);
    display_buffer[0] = '\0';

    return x_pos;
}

/**
 * @brief Render a title on the display.
 *
 * @param title The title to render.
 * @param selection The type of highlighting to apply to the title.
 */
static void render_title(const char* title, highlight_type_t selection) {
    if (!title) {
        return;
    }

    // Calculate display width needed for the text
    const char* display_text = title;

    // Account for prefix if needed
    if (selection == HIGHLIGHT_PREFIX) {
        snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "> %s", title);
        display_text = display_buffer;
    } else if (selection == HIGHLIGHT_WRAP) {
        snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "[%s]", title);
        display_text = display_buffer;
    }

    uint16_t x_pos = calculate_center_xpos(display_text);

    switch(selection) {
        case HIGHLIGHT_INVERTED:
            render_underlined_text_adv(title, x_pos, 0, 2, 1, true);
            break;
        case HIGHLIGHT_PREFIX:
            render_underlined_text(display_buffer, x_pos, 0);
            break;
        case HIGHLIGHT_WRAP:
            render_underlined_text(display_buffer, x_pos, 0);
        case HIGHLIGHT_GLYPH:
            // Not supported (fall-through)
        case HIGHLIGHT_NONE:
        default:
            render_underlined_text(title, x_pos, 0);
            break;
    }
}

/**
 * @brief Calculate the starting y-position for a group of text lines to center text on the display.
 *
 * @param num_lines The number of lines to center.
 * @param with_title Whether a title is present.
 * @param display_rows The number of rows on the display.
 * @return The y-coordinate to center the text vertically.
 */
uint16_t calculate_center_ypos_common(uint8_t num_lines, bool with_title, uint8_t display_rows) {
    uint8_t title_rows = with_title ? 2 : 0;
    uint8_t remaining_rows = display_rows - title_rows;
    uint8_t min_line = title_rows;
    uint8_t start_line = title_rows + ((remaining_rows - num_lines) / 2);
    return MAX(start_line, min_line);
}

/**
 * @brief Calculate the number of lines the content uses.
 *
 * @param content the screen content.
 * @return The number of lines the screen content uses.
 */
static uint8_t calculate_content_line_count(const screen_content_t* content) {
    uint8_t line_count = 0;
    bool in_button_row = false;

    for (uint8_t i = 0; i < content->element_count; i++) {
        const screen_element_t* element = &content->elements[i];
        if (element->type == CONTENT_TYPE_BUTTON) {
            // Buttons in a row count as 1 line total
            // and they are all adjacent
            if (!in_button_row) {
                line_count++;
                in_button_row = true;
            }
        } else {
            // Each non-button element gets its own line
            in_button_row = false;
            line_count++;
        }
    }

    return line_count;
}

/**
 * @brief Represents the x and y position that the content uses.
 */
typedef struct {
    uint8_t x;
    uint8_t y;
} element_position_t;

/**
 * @brief Calculate the coordinates for a button element.
 *
 * @param element The button element to calculate the position for.
 * @param i The index of the button element in the content.
 * @param content The screen content.
 * @param current_xpos The current x position of the button row.
 * @param center_ypos The y position to center the button row.
 */
static element_position_t calculate_button_position(screen_element_t* element, uint8_t i, screen_content_t* content, uint16_t current_xpos, uint16_t center_ypos) {
    uint8_t x, y;

    // Button rows share the same y-position
    y = element->y ? element->y : center_ypos;

    // For subsequent buttons in a row, adjust x position based on previous buttons
    if (i > 0 && content->elements[i-1].type == CONTENT_TYPE_BUTTON) {
        const list_item_t* prev = &content->elements[i-1].content.list_item;
        const char* prev_text = prev->is_dynamic ? prev->text.get_text() : prev->text.static_text;
        x = current_xpos + calculate_text_width(prev_text) + calculate_text_width(BUTTON_SEPARATOR);
    } else {
        if (element->x) {
            x = element->x;
        } else {
            // Build complete button string with spacing
            memset(display_buffer, 0, DISPLAY_BUFFER_SIZE);
            for (uint8_t j = i; j < content->element_count; j++) {
                if (content->elements[j].type == CONTENT_TYPE_BUTTON) {
                    const list_item_t* button = &content->elements[j].content.list_item;
                    const char* text = button->is_dynamic ? button->text.get_text() : button->text.static_text;

                    // Add spacing between buttons
                    if (display_buffer[0] != '\0') {
                        strcat(display_buffer, BUTTON_SEPARATOR);
                    }
                    strcat(display_buffer, text);
                } else {
                    break;  // End of button row
                }
            }

            // Now we can center the whole row if needed
            x = calculate_center_xpos(display_buffer);
        }
    }
    return (element_position_t){x, y};
}

/**
 * @brief Render the content of a screen.
 *
 * @param content The content to render.
 */
static void render_screen_content(screen_content_t* content) {
    if (!content) {
        return;
    }

    int8_t highlight_index = -1;
    if (content->get_highlight_index) {
        highlight_index = content->get_highlight_index();
    }

    // Render title if present
    if (content->title) {
        render_title(content->title, content->title_highlight);
    }

    // Calculate center position for elements if no (x, y)
    // coordinates are provided to center the elements
    // vertically, and account for title, if one is present
    uint16_t center_ypos = calculate_center_ypos(calculate_content_line_count(content), content->title);

    uint8_t current_xpos = 0;

    // Render each element
    for (uint8_t i = 0; i < content->element_count; i++) {
        screen_element_t* element = &content->elements[i];
        if (!element) {
            continue;
        }

        // Only set highlight for list items and only if they exist
        if (element->type == CONTENT_TYPE_LIST_ITEM || element->type == CONTENT_TYPE_BUTTON) {
            element->content.list_item.highlight_type =
                (highlight_index == i) ? HIGHLIGHT_INVERTED : HIGHLIGHT_NONE;
        }

        uint8_t x, y;
        if (element->type == CONTENT_TYPE_BUTTON) {
            element_position_t pos = calculate_button_position(element, i, content, current_xpos, center_ypos);
            x = pos.x;
            y = pos.y;
            current_xpos = x;
        } else {
            // Regular elements increment y-position per item
            x = element->x ? element->x : content->default_x;
            y = element->y ? element->y : center_ypos + i;

            if (content->center_contents) {
                // Adjust x for centering if needed
                x = calculate_center_position(element);
            }
        }

        switch (element->type) {
            case CONTENT_TYPE_KEY_VALUE:
                render_key_value(&element->content.key_value, x, y);
                break;

            case CONTENT_TYPE_LIST_ITEM:
                // same as button row rendering call
            case CONTENT_TYPE_BUTTON:
                render_list_item(&element->content.list_item, x, y);
                break;

            case CONTENT_TYPE_IMAGE:
                // Render image at origin if no x, y coordinates are provided
                // instead of centering (like text content)
                render_image(element->content.image,
                            element->x ? element->x : 0,
                            element->y ? element->y : 0);
                break;

            case CONTENT_TYPE_CUSTOM:
                element->content.custom_render();
                break;
        }
    }
}

/**
 * @brief Swap the top screen of the stack to change what is currently being displayed.
 *
 * This is a convenience function to immediately swap the top screen of the stack to
 * change the current screen being displayed, without having to pop the current screen,
 * and then push a new screen onto the stack. This function will fail if the provided
 * screen is NULL, the owner is NULL, the stack is full, the screen is already in the
 * stack, or the owner of the screen does not match the owner of the current screen.
 *
 * @param screen The screen to swap to.
 * @return The status of the screen swap operation.
 */
screen_push_status_t swap_screen(managed_screen_t screen) {
    if ((screen.is_custom && screen.display.render == NULL) ||
        (!screen.is_custom && screen.display.content == NULL)) {
        dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_SCREEN_NULL]);
        return SCREEN_PUSH_FAIL_SCREEN_NULL;
    }
    if (screen.owner == NULL) {
        dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_OWNER_NULL]);
        return SCREEN_PUSH_FAIL_OWNER_NULL;
    }
    if (screen_stack.top >= screen_stack.max_size) {
        dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_STACK_FULL]);
        return SCREEN_PUSH_FAIL_STACK_FULL;
    }

    // Check if screen is already in stack
    for (int i = 0; i <= screen_stack.top; i++) {
        managed_screen_t current_screen = screen_stack.screens[i];
        if ((screen.is_custom && screen.display.render == current_screen.display.render) ||
            (!screen.is_custom && screen.display.content == current_screen.display.content)) {
            dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK]);
            return SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK;
        }
    }

    if (screen_stack.top < 0) {
        screen_stack.top = 0;
    } else {
        managed_screen_t current_screen = screen_stack.screens[screen_stack.top];
        if (strcmp(current_screen.owner, screen.owner) != 0) {
            dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_OWNER_MISMATCH]);
            return SCREEN_PUSH_FAIL_OWNER_MISMATCH;
        }
    }

    clear_display();

    screen_stack.screens[screen_stack.top] = screen;
    return SCREEN_PUSH_SUCCESS;
}

/**
 * @brief Adds a screen to the stop of the stack so that it can be displayed.
 *
 * This function adds a screen to the top of the stack so that it can be
 * immediately displayed. This function will fail if the screen is NULL, the
 * owner is NULL, the stack is full, the screen is already in the stack.
 *
 * @param screen The screen to add to the stack.
 * @return The status of the screen push operation.
 */
screen_push_status_t push_screen(managed_screen_t screen) {
    if ((screen.is_custom && screen.display.render == NULL) ||
        (!screen.is_custom && screen.display.content == NULL)) {
        dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_SCREEN_NULL]);
        return SCREEN_PUSH_FAIL_SCREEN_NULL;
    }
    if (screen.owner == NULL) {
        dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_OWNER_NULL]);
        return SCREEN_PUSH_FAIL_OWNER_NULL;
    }
    if (screen_stack.top >= screen_stack.max_size) {
        dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_STACK_FULL]);
        return SCREEN_PUSH_FAIL_STACK_FULL;
    }

    // Check if screen is already in stack
    for (int i = 0; i <= screen_stack.top; i++) {
        managed_screen_t current_screen = screen_stack.screens[i];
        if ((screen.is_custom && screen.display.render == current_screen.display.render) ||
            (!screen.is_custom && screen.display.content == current_screen.display.content)) {
            dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK]);
            return SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK;
        }
    }

    clear_display();

    screen_stack.screens[++screen_stack.top] = screen;
    return SCREEN_PUSH_SUCCESS;
}

static void free_screen_memory(managed_screen_t* screen) {
    if (!screen || screen->is_const) return;

    if (!screen->is_custom && screen->display.content) {
        screen_content_t* content = screen->display.content;

        // We only free the elements array that was dynamically allocated in
        // convert_display_content(), but NOT the text content it points to
        if (content->elements) {
            free(content->elements);
            content->elements = NULL;
        }

        // Free the screen_content_t wrapper structure that was allocated
        // in convert_display_content()
        free(content);
        screen->display.content = NULL;
    }
}

/**
 * @brief Removes the top screen from the stack so that the next screen can be displayed.
 *
 * This function removes the top screen from the stack so that the next screen
 * can be displayed. The screen owner parameter must match the owner of the top
 * screen in the stack in order for it to be popped. This function will fail if
 * the stack is empty, the owner is NULL, or the owner of the screen to be popped
 * does not match the owner supplied to the pop function.
 *
 * @param owner The owner of the screen to pop.
 * @return The status of the screen pop operation.
 */
screen_pop_status_t pop_screen(const char* owner) {
    if (owner == NULL) {
        dprintf("Failed to pop screen: %s\n", screen_pop_status_strings[SCREEN_POP_FAIL_OWNER_NULL]);
        return SCREEN_POP_FAIL_OWNER_NULL;
    }

    if (screen_stack.top < 0) {
        dprintf("Failed to pop screen: %s\n", screen_pop_status_strings[SCREEN_POP_FAIL_STACK_EMPTY]);
        return SCREEN_POP_FAIL_STACK_EMPTY;
    }

    if (strcmp(screen_stack.screens[screen_stack.top].owner, owner) != 0) {
        dprintf("Failed to pop screen: %s\n", screen_pop_status_strings[SCREEN_POP_FAIL_OWNER_MISMATCH]);
        return SCREEN_POP_FAIL_OWNER_MISMATCH;
    }

    clear_display();

    // Free screen memory
    managed_screen_t* screen = &screen_stack.screens[screen_stack.top--];
    free_screen_memory(screen);
    return SCREEN_POP_SUCCESS;
}

/**
 * @brief Displays the current/top screen.
 *
 * This function displays the current/top screen. This function should be called
 * in your main keyboard loop. Consider using `housekeeping_task_user` for this.
 */
void show_current_screen(void) {
    if (screen_stack.top < 0) {
        return;
    }

    uint32_t now = timer_read32();
    managed_screen_t* current = &screen_stack.screens[screen_stack.top];

    if ((now - last_refresh) >= current->refresh_interval_ms) {
        last_refresh = now;
        if (current->is_custom) {
            current->display.render();
        } else {
            render_screen_content(current->display.content);
        }
        flush_display();
    }
}

/**
 * @brief Gets the owner of the current/top screen of the stack.
 *
 * This function returns the owner of the screen that is on the top of the stack
 * and is currently being displayed.
 *
 * @return The owner of the current screen.
 */
const char* get_current_screen_owner(void) {
    if (screen_stack.top < 0) {
        return NULL;
    }
    return screen_stack.screens[screen_stack.top].owner;
}

/**
 * @brief Gets the size of the screen stack.
 *
 * @return The size of the screen stack.
 */
uint8_t get_screen_stack_size(void) {
    return screen_stack.top + 1;
}
