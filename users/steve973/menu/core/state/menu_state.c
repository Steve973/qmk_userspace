#include <stdlib.h>
#include "quantum/logging/debug.h"
#include "menu_state.h"
#include "../operation/menu_operation.h"
#include "../operation/operation_types.h"

// Global menu state
menu_state_t menu_state;
extern const menu_item_t* const menu_root;

void init_menu_state(void) {
    if (menu_state.current != NULL || menu_state.history.depth > 0) {
        // TODO: Make sure any existing menu state is cleaned up
    }

    menu_state = (menu_state_t) {
        .current = NULL,
        .selected_index = 0,
        .timeout_ms = DEFAULT_TIMEOUT_MS,
        .show_shortcuts = false,
        .history = {
            .items = { NULL },
            .depth = 0
        }
    };
}

bool is_menu_active(void) {
    return menu_state.current != NULL;
}

nav_context_t get_current_context(void) {
    if (!is_menu_active()) {
        return NAV_CONTEXT_INVALID;
    }

    if (is_operation_in_progress()) {
        // Return context based on current operation phase
        operation_phase_t phase = get_current_operation_phase();
        switch (phase) {
            case OPERATION_PHASE_PRECONDITION:
                return NAV_CONTEXT_PRECONDITION;
            case OPERATION_PHASE_INPUT:
                return NAV_CONTEXT_INPUT;
            case OPERATION_PHASE_CONFIRMATION:
                return NAV_CONTEXT_CONFIRMATION;
            case OPERATION_PHASE_ACTION:
                return NAV_CONTEXT_ACTION;
            case OPERATION_PHASE_RESULT:
                return NAV_CONTEXT_RESULT;
            case OPERATION_PHASE_POSTCONDITION:
                return NAV_CONTEXT_POSTCONDITION;
            default:
                dprintf("Invalid operation phase: %d\n", phase);
                return NAV_CONTEXT_INVALID;
        }
    }

    return NAV_CONTEXT_MENU;
}

void set_current_menu(const menu_item_t* menu) {
    menu_state.current = menu;
    menu_state.selected_index = 0;
}

void set_selected_index(uint8_t index) {
    if (!menu_state.current) return;

    // Ensure index is within bounds
    if (menu_state.current->child_count > 0 &&
        index < menu_state.current->child_count) {
        menu_state.selected_index = index;
    }
}

int8_t get_selected_index(void) {
    return menu_state.selected_index;
}

bool is_shortcuts_enabled(void) {
    return menu_state.show_shortcuts;
}

void set_shortcuts_visible(bool visible) {
    menu_state.show_shortcuts = visible;
}

void set_menu_timeout(uint32_t timeout_ms) {
    menu_state.timeout_ms = timeout_ms;
}
