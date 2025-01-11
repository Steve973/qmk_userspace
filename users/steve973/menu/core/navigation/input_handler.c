#include <stdint.h>
#include "debug.h"
#include "../base/menu_core.h"
#include "../navigation/input_handler.h"
#include "../navigation/menu_navigation.h"
#include "../state/menu_state.h"

bool handle_menu_input(uint16_t keycode, keyrecord_t* record) {
    if (!record->event.pressed || !is_menu_active()) return false;

    nav_context_t context = get_current_context();

    switch (context) {
        case NAV_CONTEXT_MENU:
            handle_menu_navigation_input(keycode);
            break;

        case NAV_CONTEXT_INPUT:
        case NAV_CONTEXT_CONFIRMATION:
        case NAV_CONTEXT_ACTION:
        case NAV_CONTEXT_RESULT:
            handle_operation_input(keycode);
            break;

        default:
            break;
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
    bool shortcuts_enabled = is_shortcuts_enabled();

    switch (keycode) {
        case KC_W:
        case KC_UP:
            return set_selected_index((current_index + item_count - 1) % item_count);

        case KC_S:
        case KC_DOWN:
            return set_selected_index((current_index + 1) % item_count);

        case KC_D:
        case KC_ENTER:
        case KC_RIGHT:
            return menu_invoke();

        case KC_A:
        case KC_ESC:
        case KC_LEFT:
            return can_navigate_back() ? menu_return() : exit_menu_mode();

        default:
            // Handle shortcuts if enabled
            if (shortcuts_enabled) {
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

bool handle_operation_input(uint16_t keycode) {
    dprintf("Operation input: %d\n", keycode);
    // TODO: Implement operation-specific input handling
    // This would handle input during various operation phases
    return true;
}
