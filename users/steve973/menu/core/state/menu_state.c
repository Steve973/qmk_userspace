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
            .depth = -1
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
            case OPERATION_PHASE_NONE:
                dprintln("WARNING: Should not be determining operation phase for operation phase NONE");
                return NAV_CONTEXT_MENU;
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
            case OPERATION_PHASE_COMPLETE:
                dprintln("WARNING: Should not be determining operation phase for operation phase COMPLETE");
                return NAV_CONTEXT_MENU;
            default:
                dprintf("Invalid operation phase: %d\n", phase);
                return NAV_CONTEXT_INVALID;
        }
    }

    return NAV_CONTEXT_MENU;
}

bool set_current_menu(const menu_item_t* menu) {
    menu_state.current = menu;
    menu_state.selected_index = 0;
    return true;
}

bool set_selected_index(uint8_t index) {
    if (!menu_state.current) return false;

    // Ensure index is within bounds
    if (menu_state.current->child_count > 0 &&
        index < menu_state.current->child_count) {
        menu_state.selected_index = index;
        return true;
    } else {
        return false;
    }
}

bool push_menu_history(const menu_item_t* item) {
    if (menu_state.history.depth >= MAX_MENU_DEPTH) {
        dprintln("WARNING: Menu history depth exceeded!");
        return false;
    }

    // Update menu state by:
    // 1. Set new current menu item
    menu_state.current = item;
    // 2. Reset selected index
    menu_state.selected_index = 0;
    // Check if we have only just added the main menu, where the depth would be
    // -1, and the current item would be NULL, and there is nothing to add to
    // the history, so we would skip this step.
    if (menu_state.history.depth >= 0 && menu_state.current) {
        // 3. Save current menu item to history
        menu_state.history.items[menu_state.history.depth] = menu_state.current;
    }
    // 4. Increment history depth in all cases
    menu_state.history.depth++;

    return true;
}

bool pop_menu_history(void) {
    if (menu_state.history.depth < 1) {
        return false;
    }
    menu_state.current = menu_state.history.items[--menu_state.history.depth];
    menu_state.selected_index = 0;
    return true;
}

// Maybe also
uint8_t get_history_depth(void) {
    return menu_state.history.depth;
}

int8_t get_selected_index(void) {
    return menu_state.selected_index;
}

/**
 * @brief Get the current menu item.
 *
 * This function will return the current menu item that is being displayed.
 */
const menu_item_t* get_current_menu(void) {
    return menu_state.current;
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

uint32_t get_menu_timeout(void) {
    return menu_state.timeout_ms;
}
