#include <stdint.h>
#include "debug.h"
#include "../base/menu_core.h"
#include "../navigation/input_handler.h"
#include "../navigation/menu_navigation.h"
#include "../operation/menu_operation.h"
#include "../state/menu_state.h"

bool handle_menu_input(uint16_t keycode, keyrecord_t* record) {
    if (!record->event.pressed || !is_menu_active()) return false;

    nav_context_t context = get_current_navigation_context();

    if (context == NAV_CONTEXT_MENU) {
        handle_menu_navigation_input(keycode);
    } else if (context == NAV_CONTEXT_OPERATION) {
        handle_operation_input(keycode);
    }

    update_menu_activity();
    return false;
}

bool handle_menu_navigation_input(uint16_t keycode) {
    dprintf("Menu navigation input: %d\n", keycode);
    const menu_item_t* current = get_current_menu();
    if (!current) return false;

    uint8_t item_count = current->child_count;
    uint8_t current_index = get_selected_index();

    if (keycode == KC_W || keycode == KC_UP) {
        return set_selected_index((current_index + item_count - 1) % item_count);
    } else if (keycode == KC_S || keycode == KC_DOWN) {
        return set_selected_index((current_index + 1) % item_count);
    } else if (keycode == KC_D || keycode == KC_ENTER || keycode == KC_RIGHT) {
        return menu_invoke();
    } else if (keycode == KC_A || keycode == KC_ESC || keycode == KC_LEFT) {
        return can_navigate_back() ? menu_return() : exit_menu_mode();
    } else {
        // Handle shortcuts if enabled
        if (is_shortcuts_enabled()) {
            // Find item with matching shortcut keycode
            for (uint8_t i = 0; i < item_count; i++) {
                const menu_item_t* item = current->children[i];
                if (item->shortcut && keycode == item->shortcut[0]) {
                    set_selected_index(i);
                    return menu_invoke();
                }
            }
            return false;
        } else {
            return false;
        }
    }
}

/**
 * @brief Handle operation input based on the current operation phase.
 *
 * This function handles operation input based on the current operation phase and
 * phase state.  This function will return true if the input was handled, and false
 * if the input was not handled. If the phase state is awaiting input, and if the
 * operation phase is in a button selection mode, the input will be handled as a
 * button selection input. Otherwise, the input will be handled as a regular list
 * navigation input.
 */
bool handle_operation_input(uint16_t keycode) {
    dprintf("Operation input: %d\n", keycode);
    const menu_item_t* current = get_current_menu();
    if (!current) return false;

    operation_phase_t phase = get_current_operation_phase();
    char* phase_name;
    switch (phase) {
        // Handle cases where an operation is not in progress by returning
        case OPERATION_PHASE_NONE:
            phase_name = "NONE";
        case OPERATION_PHASE_COMPLETE:
            phase_name = phase_name == NULL ? "COMPLETE" : phase_name;
            dprintf("WARNING: Should not be handling operation input for operation phase: %s\n", phase_name);
            return false;
        default:
            break;
    }

    phase_state_t state = get_current_phase_state();
    bool button_selection_mode = (
        phase == OPERATION_PHASE_CONFIRMATION ||
        phase == OPERATION_PHASE_PRECONDITION ||
        phase == OPERATION_PHASE_POSTCONDITION ||
        phase == OPERATION_PHASE_RESULT
    ) && state == PHASE_STATE_AWAITING_INPUT;

    uint8_t item_count = current->child_count;
    uint8_t current_index = get_selected_index();
    uint8_t prev_index = (current_index + item_count - 1) % item_count;
    uint8_t next_index = (current_index + 1) % item_count;

    if (button_selection_mode) {
        if (keycode == KC_A || keycode == KC_LEFT) {
            return set_selected_index(prev_index);
        }
        if (keycode == KC_D || keycode == KC_RIGHT) {
            return set_selected_index(next_index);
        }
    } else {
        // Regular/list navigation mode
        if (keycode == KC_W || keycode == KC_UP) {
            return set_selected_index(prev_index);
        }
        if (keycode == KC_S || keycode == KC_DOWN) {
            return set_selected_index(next_index);
        }
    }

    // Common handlers for both modes
    if (keycode == KC_ENTER) {
        set_operation_selection(current_index);
        return true;
    }

    if (keycode == KC_ESC) {
        return cancel_operation(false);
    }

    return false;
}
