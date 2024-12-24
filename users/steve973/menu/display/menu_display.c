#include <stdlib.h>
#include "display_manager/display_manager.h"
#include "menu_display.h"
#include "../common/menu_core.h"

static screen_content_t* create_submenu_screen(const menu_item_t* menu_item) {
    if (!menu_item->children || menu_item->child_count == 0) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t) * menu_item->child_count);
    if (!elements) return NULL;

    // Create list items for each child menu item
    for (uint8_t i = 0; i < menu_item->child_count; i++) {
        const menu_item_t* child = menu_item->children[i];
        elements[i] = (screen_element_t) {
            .type = CONTENT_TYPE_LIST,
            .x = child->icon ? 1 : 0,  // Leave space for icon if present
            .y = i + 2,                // Start below title
            .content.list_item = {
                .text.static_text = child->label,
                .is_dynamic = false,
                .selection_type = SELECTION_PREFIX,
                .selection.prefix_char = '>'
                // TODO: Handle icons if needed
            }
        };
    }

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = menu_item->label,
        .elements = elements,
        .element_count = menu_item->child_count,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_action_screen(const menu_item_t* menu_item) {
    uint8_t element_count = 0;
    if (menu_item->operation.inputs) {
        element_count += menu_item->operation.input_count;
    }
    if (menu_item->operation.confirm) {
        element_count++;
    }

    if (element_count == 0) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t) * element_count);
    if (!elements) return NULL;

    uint8_t current_element = 0;
    uint8_t current_y = 2;

    // Input fields
    if (menu_item->operation.inputs) {
        for (uint8_t i = 0; i < menu_item->operation.input_count; i++) {
            const input_config_t* input = &menu_item->operation.inputs[i];
            elements[current_element++] = (screen_element_t) {
                .type = CONTENT_TYPE_KEY_VALUE,
                .x = 0,
                .y = current_y++,
                .content.key_value = {
                    .label = input->prompt,
                    .value.static_value = input->default_val,
                    .is_dynamic = false  // TODO: Make dynamic based on input state
                }
            };
        }
    }

    // Confirmation message if present
    if (menu_item->operation.confirm) {
        elements[current_element++] = (screen_element_t) {
            .type = CONTENT_TYPE_LIST,
            .x = 0,
            .y = current_y++,
            .content.list_item = {
                .text.static_text = menu_item->operation.confirm->message,
                .is_dynamic = false,
                .selection_type = SELECTION_NONE
            }
        };
    }

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = menu_item->label,
        .elements = elements,
        .element_count = current_element,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_display_screen(const menu_item_t* menu_item) {
    // TODO: Handle display screens based on operation.action
    // For now return NULL
    return NULL;
}

screen_content_t* create_menu_screen(const menu_item_t* menu_item) {
    if (!menu_item) return NULL;

    switch (menu_item->type) {
        case MENU_TYPE_SUBMENU:
            return create_submenu_screen(menu_item);
        case MENU_TYPE_ACTION:
            return create_action_screen(menu_item);
        case MENU_TYPE_DISPLAY:
            return create_display_screen(menu_item);
        default:
            return NULL;
    }
}

static screen_content_t* create_precondition_screen(const menu_item_t* item) {
    if (!item->operation.precondition) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t));
    if (!elements) return NULL;

    elements[0] = (screen_element_t) {
        .type = CONTENT_TYPE_LIST,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text.static_text = item->operation.precondition->message,
            .is_dynamic = false,
            .selection_type = SELECTION_NONE
        }
    };

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = item->label,
        .elements = elements,
        .element_count = 1,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_input_screen(const menu_item_t* item) {
    if (!item->operation.inputs || !item->operation.input_count) return NULL;

    uint8_t element_count = item->operation.input_count;
    if (item->operation.input_count > 1) element_count++; // Add count display

    screen_element_t* elements = malloc(sizeof(screen_element_t) * element_count);
    if (!elements) return NULL;

    uint8_t current_element = 0;
    uint8_t current_y = 2;

    // If multiple inputs, show counter
    if (item->operation.input_count > 1) {
        elements[current_element++] = (screen_element_t) {
            .type = CONTENT_TYPE_LIST,
            .x = 0,
            .y = current_y++,
            .content.list_item = {
                .text.static_text = "Input X of Y", // TODO: Make dynamic
                .is_dynamic = false,
                .selection_type = SELECTION_NONE
            }
        };
    }

    // Create elements for each input
    for (uint8_t i = 0; i < item->operation.input_count; i++) {
        const input_config_t* input = &item->operation.inputs[i];
        elements[current_element++] = (screen_element_t) {
            .type = CONTENT_TYPE_KEY_VALUE,
            .x = 0,
            .y = current_y++,
            .content.key_value = {
                .label = input->prompt,
                .value.static_value = input->default_val,
                .is_dynamic = true // Will need dynamic handler for live updates
            }
        };
    }

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = item->label,
        .elements = elements,
        .element_count = current_element,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_confirm_screen(const menu_item_t* item) {
    if (!item->operation.confirm) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t) * 2); // Message + options
    if (!elements) return NULL;

    // Confirmation message
    elements[0] = (screen_element_t) {
        .type = CONTENT_TYPE_LIST,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text.static_text = item->operation.confirm->message,
            .is_dynamic = false,
            .selection_type = SELECTION_NONE
        }
    };

    // Yes/No options
    elements[1] = (screen_element_t) {
        .type = CONTENT_TYPE_LIST,
        .x = 0,
        .y = 4,
        .content.list_item = {
            .text.static_text = item->operation.confirm->true_text,
            .is_dynamic = false,
            .selection_type = SELECTION_PREFIX,
            .selection.prefix_char = '>'
        }
    };

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = item->label,
        .elements = elements,
        .element_count = 2,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_result_screen(const menu_item_t* item) {
    if (!item->operation.result) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t));
    if (!elements) return NULL;

    elements[0] = (screen_element_t) {
        .type = CONTENT_TYPE_LIST,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text.static_text = item->operation.result->message,
            .is_dynamic = false,
            .selection_type = SELECTION_NONE
        }
    };

    // If it's an acknowledge mode result, add the OK button
    if (item->operation.result->mode == RESULT_MODE_ACKNOWLEDGE) {
        screen_element_t* new_elements = realloc(elements, sizeof(screen_element_t) * 2);
        if (!new_elements) {
            free(elements);
            return NULL;
        }
        elements = new_elements;

        elements[1] = (screen_element_t) {
            .type = CONTENT_TYPE_LIST,
            .x = 0,
            .y = 4,
            .content.list_item = {
                .text.static_text = item->operation.result->ok_text,
                .is_dynamic = false,
                .selection_type = SELECTION_PREFIX,
                .selection.prefix_char = '>'
            }
        };
    }

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = item->label,
        .elements = elements,
        .element_count = (item->operation.result->mode == RESULT_MODE_ACKNOWLEDGE) ? 2 : 1,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

static screen_content_t* create_postcondition_screen(const menu_item_t* item) {
    if (!item->operation.postcondition) return NULL;

    screen_element_t* elements = malloc(sizeof(screen_element_t));
    if (!elements) return NULL;

    elements[0] = (screen_element_t) {
        .type = CONTENT_TYPE_LIST,
        .x = 0,
        .y = 2,
        .content.list_item = {
            .text.static_text = item->operation.postcondition->message,
            .is_dynamic = false,
            .selection_type = SELECTION_NONE
        }
    };

    screen_content_t* screen = malloc(sizeof(screen_content_t));
    if (!screen) {
        free(elements);
        return NULL;
    }

    *screen = (screen_content_t) {
        .title = item->label,
        .elements = elements,
        .element_count = 1,
        .default_y = 2,
        .center_contents = false
    };

    return screen;
}

screen_content_t* create_operation_screen(const menu_item_t* item, operation_phase_t phase) {
    if (!item) return NULL;

    switch (phase) {
        case OPERATION_PHASE_PRECONDITION:
            return create_precondition_screen(item);
        case OPERATION_PHASE_INPUT:
            return create_input_screen(item);
        case OPERATION_PHASE_CONFIRMATION:
            return create_confirm_screen(item);
        case OPERATION_PHASE_RESULT:
            return create_result_screen(item);
        case OPERATION_PHASE_POSTCONDITION:
            return create_postcondition_screen(item);
        default:
            return NULL;
    }
}

void free_menu_screen(screen_content_t* screen) {
    if (screen) {
        if (screen->elements) {
            free(screen->elements);
        }
        free(screen);
    }
}
