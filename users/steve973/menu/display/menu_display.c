#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <progmem.h>
#include <action.h>
#include "debug.h"
#include "display_manager/display_manager.h"
#include "menu_display.h"
#include "../common/menu_core.h"

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

screen_content_t* create_menu_screen(const menu_item_t* menu_item) {
    if (!menu_item) {
        dprintf("create_menu_screen: menu_item is NULL\n");
        return NULL;
    }
    if (!menu_item->display) {
        dprintf("create_menu_screen: menu_item->display is NULL\n");
        return NULL;
    }
    dprintf("Creating menu screen for: %s\n", menu_item->label);
    return convert_display_content(menu_item->display);
}

screen_content_t* create_operation_screen(const menu_item_t* item, operation_phase_t phase) {
    if (!item) return NULL;

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
            return NULL;
    }

    return convert_display_content(display);
}

void free_menu_screen(screen_content_t* screen) {
    if (screen) {
        if (screen->elements) {
            free(screen->elements);
        }
        free(screen);
    }
}
