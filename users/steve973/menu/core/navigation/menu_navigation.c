#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "menu_navigation.h"
#include "../state/menu_state.h"
#include "../operation/menu_operation.h"
#include "../structure/menu_item.h"
#include "../../display/menu_display.h"

extern const menu_item_t* const menu_root;

/**
 * @brief Push a menu item onto the "stack".
 *
 * This function will push the provided menu item onto the "stack" of menu items,
 * as long as the stack is not full.  The stack is used to track the navigation
 * history so that the user can navigate back through the menu to the previous
 * menu item.
 *
 * @param item The menu item to push onto the stack.
 */
static bool menu_push_item(const menu_item_t* item) {
    if (!item) {
        return false;
    }
    screen_push_status_t push_status = create_menu_screen(item, &get_selected_index, MENU_OWNER);
    return push_status == SCREEN_PUSH_SUCCESS ? push_menu_history(item) : false;
}

/**
 * @brief Pop a menu item from the "stack".
 *
 * This function will pop the current menu item from the "stack" of menu items,
 * as long as the stack is not empty, and as long as the current screen is owned
 * by the menu system.  The stack is used to track the navigation history so
 * that the user can navigate back through the menu to the previous menu item.
 */
static bool menu_pop_item(void) {
    if (strcmp(get_current_screen_owner(), MENU_OWNER) != 0) {
        dprintln("Current screen cannot be popped since it is not owned by the menu system");
        return false;
    }
    screen_pop_status_t pop_result = remove_menu_screen(MENU_OWNER);
    return pop_result == SCREEN_POP_SUCCESS ? pop_menu_history() : false;
}

/**
 * @brief Invoke the selected menu item.
 *
 * This function will invoke the selected menu item, which means that it will
 * show the item if it is a submenu, or it will start the operation if it is an
 * operation.
 */
bool menu_invoke(void) {
    const menu_item_t* current = get_current_menu();
    uint8_t selected_index = get_selected_index();
    if (!current || !current->children || selected_index >= current->child_count) {
        return false;
    }

    const menu_item_t* selected = current->children[selected_index];

    switch (selected->type) {
        case MENU_TYPE_ACTION:
            return selected->operation.action ? start_operation(selected) : false;
        case MENU_TYPE_DISPLAY:
            return menu_push_item(selected);
        case MENU_TYPE_SUBMENU:
            return menu_push_item(selected);
        default:
            return false;
    }
}

/**
 * @brief Return from the current menu.
 *
 * This function will return from the current menu, which means that it will
 * navigate back to the previous menu item in the navigation history. This
 * function does not exit the menu if the current menu is the home screen.
 */
bool menu_return(void) {
    dprintln("Returning from current menu");
    if (is_operation_in_progress()) {
        return cancel_operation(false);
    } else if (get_history_depth() > 0) {
        return menu_pop_item();
    }
    return false;
}

/**
 * @brief Go to the menu home screen.
 *
 * This function will initialize the menu state and go to the menu home screen.
 */
bool menu_home(void) {
    init_menu_state();
    return menu_push_item(menu_root);
}

/**
 * @brief Check if it is possible to navigate back in the menu.
 *
 * This function will return true if it is possible to navigate back in the menu,
 * which means that the navigation history is not empty, or that an operation is
 * in progress.
 */
bool can_navigate_back(void) {
    return get_history_depth() > 0 || is_operation_in_progress();
}
