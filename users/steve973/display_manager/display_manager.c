#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "debug.h"
#include "display_manager.h"
#include "timer.h"

static screen_stack_t screen_stack = {
    .top = -1
};

static uint32_t last_refresh = 0;

static const char* const screen_push_status_strings[] = {
    [SCREEN_PUSH_SUCCESS] = "SCREEN_PUSH_SUCCESS",
    [SCREEN_PUSH_FAIL_SCREEN_NULL] = "SCREEN_PUSH_FAIL_SCREEN_NULL",
    [SCREEN_PUSH_FAIL_OWNER_NULL] = "SCREEN_PUSH_FAIL_OWNER_NULL",
    [SCREEN_PUSH_FAIL_STACK_FULL] = "SCREEN_PUSH_FAIL_STACK_FULL",
    [SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK] = "SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK",
    [SCREEN_PUSH_FAIL_OWNER_MISMATCH] = "SCREEN_PUSH_FAIL_OWNER_MISMATCH"
};

static const char* const screen_pop_status_strings[] = {
    [SCREEN_POP_SUCCESS] = "SCREEN_POP_SUCCESS",
    [SCREEN_POP_FAIL_STACK_EMPTY] = "SCREEN_POP_FAIL_STACK_EMPTY",
    [SCREEN_POP_FAIL_OWNER_NULL] = "SCREEN_POP_FAIL_OWNER_NULL",
    [SCREEN_POP_FAIL_OWNER_MISMATCH] = "SCREEN_POP_FAIL_OWNER_MISMATCH",
    [SCREEN_POP_FAIL_SCREEN_NOT_IN_STACK] = "SCREEN_POP_FAIL_SCREEN_NOT_IN_STACK"
};

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

    // Render each element
    for (uint8_t i = 0; i < content->element_count; i++) {
        screen_element_t* element = &content->elements[i];
        if (!element) {
            continue;
        }

        // Only set highlight for list items and only if they exist
        if (element->type == CONTENT_TYPE_LIST) {
            element->content.list_item.highlight_type =
                (highlight_index == i) ? HIGHLIGHT_INVERTED : HIGHLIGHT_NONE;
        }

        // Use default position if element position is 0,0
        uint8_t x = element->x ? element->x : content->default_x;
        uint8_t y = element->y ? element->y : content->default_y;

        if (content->center_contents) {
            // Adjust x for centering if needed
            x = calculate_center_position(element);
        }

        switch (element->type) {
            case CONTENT_TYPE_KEY_VALUE:
                render_key_value(&element->content.key_value, x, y);
                break;

            case CONTENT_TYPE_LIST:
                render_list_item(&element->content.list_item, x, y);
                break;

            case CONTENT_TYPE_IMAGE:
                render_image(&element->content.image, x, y);
                break;

            case CONTENT_TYPE_CUSTOM:
                element->content.custom_render();
                break;
        }
    }
}

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
    if (screen_stack.top >= 9) {
        dprintf("Failed to swap screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_STACK_FULL]);
        return SCREEN_PUSH_FAIL_STACK_FULL;
    }

    clear_display();

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

    screen_stack.screens[screen_stack.top] = screen;
    return SCREEN_PUSH_SUCCESS;
}

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
    if (screen_stack.top >= 9) {
        dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_STACK_FULL]);
        return SCREEN_PUSH_FAIL_STACK_FULL;
    }

    clear_display();

    // Check if screen is already in stack
    for (int i = 0; i <= screen_stack.top; i++) {
        managed_screen_t current_screen = screen_stack.screens[i];
        if ((screen.is_custom && screen.display.render == current_screen.display.render) ||
            (!screen.is_custom && screen.display.content == current_screen.display.content)) {
            dprintf("Failed to push screen: %s\n", screen_push_status_strings[SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK]);
            return SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK;
        }
    }

    screen_stack.screens[++screen_stack.top] = screen;
    return SCREEN_PUSH_SUCCESS;
}

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

    --screen_stack.top;
    return SCREEN_POP_SUCCESS;
}

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

const char* get_current_screen_owner(void) {
    if (screen_stack.top < 0) {
        return NULL;
    }
    return screen_stack.screens[screen_stack.top].owner;
}

uint8_t get_screen_stack_size(void) {
    return screen_stack.top + 1;
}
