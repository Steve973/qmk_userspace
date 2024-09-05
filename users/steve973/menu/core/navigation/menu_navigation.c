#include <stdbool.h>
#include "menu_navigation.h"
#include "display_manager/display_manager.h"
#include "../state/menu_state.h"
#include "../operation/menu_operation.h"
#include "../structure/menu_item.h"
#include "../../display/menu_display.h"
#include "menu/core/base/menu_core.h"

extern menu_state_t menu_state;
extern const menu_item_t* const menu_root;

bool menu_pop() {
    if(pop_screen(MENU_OWNER) && menu_state.history.depth > 0) {
        menu_state.current = menu_state.history.items[--menu_state.history.depth];
        menu_state.selected_index = 0;
        return true;
    }
    return false;
}

void menu_home(void) {
    // Initialize menu state
    init_menu_state();

    menu_state.current = menu_root;

    // Create and push new menu screen
    screen_content_t* screen = create_menu_screen(menu_state.current);
    if (screen) {
        screen->get_highlight_index = &get_selected_index;
        push_screen((managed_screen_t){
            .owner = MENU_OWNER,
            .is_custom = false,
            .display.content = screen,
            .refresh_interval_ms = 0
        });
    }
}

bool menu_enter(void) {
    const menu_item_t* current = menu_state.current;
    if (!current || !current->children ||
        menu_state.selected_index >= current->child_count) {
        return false;
    }

    const menu_item_t* selected = current->children[menu_state.selected_index];

    // Handle submenu navigation
    if (selected->type == MENU_TYPE_SUBMENU) {
        if (!selected->children || selected->child_count == 0) {
            return false;
        }

        // Save current position in history
        if (menu_state.history.depth < MAX_MENU_DEPTH) {
            menu_state.history.items[menu_state.history.depth++] = current;
        }

        // Create and push new submenu screen
        screen_content_t* screen = create_menu_screen(selected);
        if (screen) {
            screen->get_highlight_index = &get_selected_index;
            push_screen((managed_screen_t){
                .owner = MENU_OWNER,
                .is_custom = false,
                .display.content = screen,
                .refresh_interval_ms = 0
            });
        }

        // Update menu state
        menu_state.current = selected;
        menu_state.selected_index = 0;
        return true;
    }

    // Handle action items
    else if (selected->type == MENU_TYPE_ACTION && selected->operation.action) {
        menu_state.current = selected;
        start_operation(selected);
        return true;
    }

    return false;
}

bool menu_back(void) {
    // Handle operation in progress
    if (is_operation_in_progress()) {
        cancel_operation();
        return true;
    }

    // Handle menu navigation
    if (menu_state.history.depth > 0) {
        menu_pop();
        return true;
    } else {
        exit_menu_mode();
        return true;
    }

    return false;
}

bool can_navigate_back(void) {
    return menu_state.history.depth > 0 || is_operation_in_progress();
}

const menu_item_t* get_current_menu(void) {
    return menu_state.current;
}
