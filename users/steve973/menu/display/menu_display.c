#include <stdlib.h>
#include "display_manager/display_manager.h"
#include "menu_display.h"
#include "../common/menu_core.h"

static screen_content_t* convert_display_content(const display_content_t* display) {
    if (!display) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t) * display->element_count);
    if (!elements) return NULL;

    for (uint8_t i = 0; i < display->element_count; i++) {
        const display_element_t* src = &display->elements[i];
        elements[i] = (screen_element_t) {
            .type = src->type == DISPLAY_TYPE_INPUT ? CONTENT_TYPE_KEY_VALUE : CONTENT_TYPE_LIST,
            .x = 0,
            .y = i + 2,
            .content.list_item = {
                .text.static_text = src->text,
                .is_dynamic = false,
                .highlight_type = src->is_selectable ? HIGHLIGHT_INVERTED : HIGHLIGHT_NONE,
                .highlight.prefix_char = '>' // TODO: Make this configurable
            }
        };
    }

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = display->title,
        .elements = elements,
        .element_count = display->element_count,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

screen_content_t* create_menu_screen(const menu_item_t* menu_item) {
    if (!menu_item) return NULL;
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
