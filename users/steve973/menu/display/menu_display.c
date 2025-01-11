#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <progmem.h>
#include <action.h>
#include "quantum/logging/debug.h"
#include "display_manager/display_manager.h"
#include "menu_display.h"

static screen_content_t* convert_display_content(const display_content_t* display) {
    if (!display) {
        dprintf("convert_display_content: display is NULL\n");
        return NULL;
    }
    dprintf("Converting display content with %d elements\n", display->element_count);

    // First allocate screen_content
    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) return NULL;

    // Then allocate elements array
    screen_element_t* elements = NULL;
    if (display->element_count > 0) {
        elements = malloc(sizeof(screen_element_t) * display->element_count);
        if (!elements) {
            free(screen);
            return NULL;
        }

        // Convert each display element to screen element
        for (uint8_t i = 0; i < display->element_count; i++) {
            const display_element_t* src = &display->elements[i];
            elements[i] = (screen_element_t){
                .type = src->is_selectable ? CONTENT_TYPE_LIST : CONTENT_TYPE_KEY_VALUE,
                .x = 0,
                .y = i + 2,
                .content = {
                    .list_item = {
                        .text = {
                            .static_text = src->text
                        },
                        .is_dynamic = false,
                        .highlight_type = src->is_selectable ? HIGHLIGHT_INVERTED : HIGHLIGHT_NONE
                    }
                }
            };
        }
    }

    *screen = (screen_content_t){
        .title = display->title,
        .elements = elements,
        .element_count = display->element_count,
        .default_y = 2,
        .center_contents = false,
        // Initialized to null, but will be set by menu core if the
        // screen content has selectable elements
        .get_highlight_index = NULL
    };

    return screen;
}

screen_push_status_t create_menu_screen(const menu_item_t* menu_item, int8_t (*get_highlight_index)(void), const char* owner) {
    if (!menu_item) {
        dprintf("create_menu_screen: menu_item is NULL\n");
        return SCREEN_PUSH_FAIL_SCREEN_NULL;
    }
    if (!menu_item->display) {
        dprintf("create_menu_screen: menu_item->display is NULL\n");
        return SCREEN_PUSH_FAIL_SCREEN_NULL;
    }
    dprintf("Creating menu screen for: %s\n", menu_item->label);
    screen_content_t* screen = convert_display_content(menu_item->display);
    screen->get_highlight_index = get_highlight_index;
    return push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });
}

screen_push_status_t create_operation_screen(const menu_item_t* item, operation_phase_t phase, const char* owner) {
    if (!item || !item->display) return SCREEN_PUSH_FAIL_SCREEN_NULL;

    dprintf("Creating operation screen for: %s, phase: %d\n", item->label, phase);

    const display_content_t* display = NULL;
    switch (phase) {
        case OPERATION_PHASE_PRECONDITION:
            display = item->operation.precondition_display;
            break;
        case OPERATION_PHASE_INPUT:
            display = item->operation.input_display;
            break;
        case OPERATION_PHASE_CONFIRMATION:
            display = item->operation.confirm_display;
            break;
        case OPERATION_PHASE_ACTION:
            display = item->operation.action_display;
            break;
        case OPERATION_PHASE_RESULT:
            display = item->operation.result_display;
            break;
        case OPERATION_PHASE_POSTCONDITION:
            display = item->operation.postcondition_display;
            break;
        default:
            dprintf("Operation screen creation failed: invalid operation phase: %d\n", phase);
            return SCREEN_PUSH_FAIL_SCREEN_NULL;
    }

    screen_content_t* screen = convert_display_content(display);
    return push_screen((managed_screen_t){
        .owner = owner,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });
}

screen_pop_status_t remove_menu_screen(const char* owner) {
    return pop_screen(owner);
}
