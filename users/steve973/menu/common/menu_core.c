/* Copyright 2024 Sadek Baroudi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <progmem.h>
#include <action.h>
#include <sys/unistd.h>
#include "debug.h"
#include "display_manager/display_manager.h"
#include "keycodes.h"
#include "src/fp.h"
#include "timer.h"
#include "src/fp_rgb_common.h"
#include "timeout_indicator/timeout_indicator.h"
#include "../display/menu_display.h"
#include "menu/common/menu_operation.h"
#include "menu/common/menu_core.h"

#define ITEMS_PER_PAGE 10
#define DEFAULT_TIMEOUT_MS 30000

// History for menuback navigation
typedef struct {
    const menu_item_t* items[MAX_MENU_DEPTH];
    uint8_t depth;
} menu_history_t;

/* Menu State Management - Internal */
typedef struct menu_state {
    const menu_item_t* current;      // Current menu being displayed
    uint8_t selected_index;          // Currently selected item
    uint32_t last_activity;          // For timeout tracking
    uint32_t timeout_ms;             // Configurable timeout
    bool show_shortcuts;             // Show keyboard shortcuts
    menu_history_t history;          // Navigation history

    // Operation state
    struct {
        const menu_item_t* item;     // Item being operated on
        void* input_value;           // Current input value
        void* previous_value;        // Previous value
        bool in_progress;            // Operation in progress
    } operation;
} menu_state_t;

/* Static Variables */
static bool menu_active = false;
static menu_state_t menu_state;
static uint8_t menu_timeout_token = INVALID_DEFERRED_TOKEN;

extern const menu_item_t* const menu_root;  // Defined in generated code

/* Menu Activity Management */
static void update_menu_activity(void) {
    timeout_indicator_reset(menu_timeout_token);
    menu_state.last_activity = timer_read32();
}

/**
 * @brief Toggles the RGB matrix lighting to indicate to indicate menu mode.
 *
 * When invoked with "true", indicating that we are entering menu mode, this function
 * saves the current RGB matrix state and sets all keys to "HSV_BLUE" with a "breathing"
 * animation.  When invoked with "false", indicating that we are leaving menu mode, this
 * function restores the RGB matrix state to the saved state.
 *
 * @param enabled True to enable menu mode lighting, false to return to saved lighting.
 */
void set_menu_mode_lighting(bool enabled) {
    #ifdef RGB_MATRIX_ENABLE
    if (enabled) {
        // Temporarily change all keys to blue and "breathing" animation
        // to indicate that we are in menu mode and the keys are "asleep"
        fp_rgb_set_hsv_and_mode(HSV_BLUE, RGB_MATRIX_BREATHING);
    } else {
        // Restore original keyboard LED colors
        fp_rgb_set_hsv_and_mode(
            fp_config.rgb_hue,
            fp_config.rgb_sat,
            fp_config.rgb_val,
            fp_config.rgb_mode
        );
    }
    #endif
}

/**
 * @brief Initialize the menu to a clean state for entering or exiting menu mode.
 *
 * This function is called when entering or exiting menu mode to ensure that the
 * menu state is clean and ready for use.  It will pop any existing menu screens
 * from the display stack and reset the menu state to the root menu with an empty
 * history stack.
 */
bool menu_init(void) {
    while (strcmp(get_current_screen_owner(), MENU_OWNER) == 0) {
        pop_screen(MENU_OWNER);
    }

    menu_state = (menu_state_t) {
        .current = menu_root,
        .selected_index = 0,
        .last_activity = timer_read32(),
        .timeout_ms = DEFAULT_TIMEOUT_MS,
        .show_shortcuts = false,
        .history = {
            .items = { NULL },
            .depth = 0
        },
        .operation = {
            .in_progress = false,
            .item = NULL,
            .input_value = NULL,
            .previous_value = NULL
        },
    };

    return true;
}

/**
 * @brief Returns the current menu state -- true if the menu is active, false otherwise.
 */
bool is_menu_active(void) {
    return menu_active;
}

/**
 * @brief Exits menu mode.
 */
void menu_exit(void) {
    set_menu_active(false);
}

/**
 * @brief Returns the current selected/highlighted menu item index.
 */
int8_t get_current_highlight_index(void) {
    return menu_state.selected_index;
}

/**
 * @brief Returns to the menu "home" screen.
 *
 * This function ensures that the menu is in a clean state and then displays the
 * home screen of the menu. This is the entry point for the menu system, and it
 * is often called when entering menu mode or when the user presses the "home"
 * button to return to the top-level menu.
 */
void menu_home(void) {
    menu_init();

    // Create and push new submenu screen
    screen_content_t* screen = create_menu_screen(menu_state.current);

    // Provide the function to get the current highlight index
    // for the display manager to use for showing the selected
    // item in the menu
    screen->get_highlight_index = &get_current_highlight_index;

    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });
}

/**
 * @brief Sets the menu active or inactive.
 *
 * This function is used to toggle the menu mode on or off.  When the menu is
 * activated by passing "true" to the function, the menu home screen is displayed,
 * and the keyboard is locked into menu mode until the menu is deactivated,
 * either by calling this function with "false" or by timing out.  When the menu
 * is deactivated, the menu state is cleaned up, the keyboard is cleared, and
 * menu mode is exited, returning the keyboard to normal operation.
 *
 * Keyboard lighting reflects the menu mode state, with a blue "breathing"
 * animation indicating that the keyboard is in menu mode.
 *
 * @param active True to activate the menu, false to deactivate it.
 */
void set_menu_active(bool active) {
    if (active && !menu_active) {
        menu_active = true;
        menu_home();
        menu_timeout_token = timeout_indicator_create(menu_state.timeout_ms, &menu_exit);
    } else if (!active && menu_active) {
        timeout_indicator_cancel(menu_timeout_token);
        clear_keyboard();
        menu_init();
        menu_active = false;
    }
    set_menu_mode_lighting(active);
}

/**
 * @brief Enter a submenu, or execute an operation, depending on the menu selection.
 *
 * This function is called when the user selects a menu item to either enter a
 * submenu or execute an operation.  If the selected item is a submenu, the
 * function will navigate into the submenu and display the submenu screen.  If
 * the selected item is an operation, the function will execute the operation.
 *
 * @return True if the operation was successful, false otherwise.
 */
bool menu_enter(void) {
    const menu_item_t* current = menu_state.current;
    if (!current || !current->children || menu_state.selected_index >= current->child_count) {
        return false;
    }

    // Get the selected item
    const menu_item_t* selected = current->children[menu_state.selected_index];

    // If we're at a submenu, navigate into it
    if (selected->type == MENU_TYPE_SUBMENU) {
        if (!selected->children || selected->child_count == 0) {
            return false;  // Empty submenu
        }

        // Save current position in history
        if (menu_state.history.depth < MAX_MENU_DEPTH) {
            menu_state.history.items[menu_state.history.depth++] = current;
        }

        // Create and push new submenu screen
        screen_content_t* screen = create_menu_screen(selected);

        // Provide the function to get the current highlight index
        // for the display manager to use for showing the selected
        // item in the menu
        screen->get_highlight_index = &get_current_highlight_index;

        push_screen((managed_screen_t){
            .owner = MENU_OWNER,
            .is_custom = false,
            .display.content = screen,
            .refresh_interval_ms = 0
        });

        // Move to first item in submenu
        menu_state.current = selected;
        menu_state.selected_index = 0;
        return true;
    }

    // If it's an action, execute it
    else if (selected->type == MENU_TYPE_ACTION && selected->operation.action) {
        menu_state.current = selected;
        execute_operation(selected);
        return true;
    }

    return false;
}

/**
 * @brief Returns to the previous menu screen or exits the menu if at the top level.
 *
 * This function is called when the user presses the "back" button to return to
 * the previous menu screen.  If the current screen is the top-level menu, the
 * function will exit the menu mode and return the keyboard to normal operation.
 *
 * @return True if the operation was successful, false otherwise.
 */
bool menu_back(void) {
    if (menu_state.operation.in_progress) {
        if (menu_state.operation.input_value) {
            free(menu_state.operation.input_value);
        }
        if (menu_state.operation.previous_value) {
            free(menu_state.operation.previous_value);
        }
        menu_state.operation.in_progress = false;
        menu_state.operation.item = NULL;
        menu_state.operation.input_value = NULL;
        menu_state.operation.previous_value = NULL;
        return true;
    }

    if (menu_state.history.depth > 0) {
        pop_screen(MENU_OWNER);
        menu_state.current = menu_state.history.items[--menu_state.history.depth];
        menu_state.selected_index = 0;
       return true;
    }

    return false;
}

/**
 * @brief Handle menu navigation based on the keycode and navigation context.
 *
 * This function is called when a key is pressed to navigate the menu.  The function
 * will handle the navigation based on the keycode and the current navigation context.
 *
 * @param keycode The keycode of the key that was pressed.
 * @param context The current navigation context.
 */
static void handle_navigation(uint16_t keycode, nav_context_t context) {
    uint8_t item_count;
    uint8_t* selected_index;

    // Set up context-specific vars
    if (context == NAV_CONTEXT_MENU) {
        if (!menu_state.current) {
            return;
        }
        item_count = menu_state.current->child_count;
        selected_index = &menu_state.selected_index;
        dprintf("Menu navigation: items=%d, current_index=%d\n",
                item_count, *selected_index);
    } else {
        if (!menu_state.operation.item) {
            return;
        }
        item_count = menu_state.operation.item->operation.input_count;
        selected_index = &menu_state.selected_index;
    }

    switch (keycode) {
        case KC_W:
        case KC_UP:
            if (*selected_index > 0) {
                (*selected_index)--;
            } else {
                *selected_index = item_count - 1;
            }
            break;

        case KC_S:
        case KC_DOWN:
            if (*selected_index < item_count - 1) {
                (*selected_index)++;
            } else {
                *selected_index = 0;
            }
            break;

        case KC_D:
        case KC_ENTER:
        case KC_RIGHT:
            if (context == NAV_CONTEXT_MENU) {
                menu_enter();
            }
            break;

        case KC_A:
        case KC_ESC:
        case KC_LEFT:
            if (context == NAV_CONTEXT_MENU) {
                if (menu_state.history.depth == 0) {
                    set_menu_active(false);
                } else {
                    menu_back();
                }
            }
            break;

        default:
            if (context == NAV_CONTEXT_MENU &&
                menu_state.show_shortcuts &&
                menu_state.current->children) {
                // Handle shortcuts
                for (uint8_t i = 0; i < menu_state.current->child_count; i++) {
                    const menu_item_t* item = menu_state.current->children[i];
                    if (item->shortcut && keycode == item->shortcut[0]) {
                        menu_state.selected_index = i;
                        menu_enter();
                    }
                }
            }
            break;
    }
}

/**
 * @brief Process a key press event for the menu system.
 *
 * This function is called when a key is pressed to process the key press event
 * for the menu system.  The function determines the navigation context, and
 * this means that, if an operation is in progress, it determines the current
 * operation phase, or it sets the phase to indicate that the key was pressed
 * as part of normal menu navigation.  At this point, this function calls the
 * function to process naviagation with the key and context, then updates the
 * menu activity to reset the timeout indicator.
 *
 * @param keycode The keycode of the key that was pressed.
 * @param record The key record for the key press event.
 * @return True to further process the keypress, false otherwise.
 */
bool process_menu_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return false;

    const menu_item_t* current = menu_state.current;
    if (!current) return false;

    nav_context_t context = NAV_CONTEXT_INVALID;

    if (is_operation_in_progress()) {
        operation_phase_t operation_phase = get_current_operation_phase();
        switch (operation_phase) {
            case OPERATION_PHASE_PRECONDITION:
                context = NAV_CONTEXT_PRECONDITION;
                break;
            case OPERATION_PHASE_INPUT:
                context = NAV_CONTEXT_INPUT;
                break;
            case OPERATION_PHASE_CONFIRMATION:
                context = NAV_CONTEXT_CONFIRMATION;
                break;
            case OPERATION_PHASE_ACTION:
                context = NAV_CONTEXT_ACTION;
                break;
            case OPERATION_PHASE_RESULT:
                context = NAV_CONTEXT_RESULT;
                break;
            case OPERATION_PHASE_POSTCONDITION:
                context = NAV_CONTEXT_POSTCONDITION;
                break;
            default:
                dprintf("Invalid operation phase: %d\n", operation_phase);
                break;
        }
    } else {
        context = NAV_CONTEXT_MENU;
    }

    handle_navigation(keycode, context);
    update_menu_activity();

    return false;
}

/**
 * @brief Create a new menu item.
 *
 * This function creates a new menu item with the specified label, short label,
 * and type.  The function allocates memory for the new menu item and initializes
 * the item with the specified values.  The function returns a pointer to the
 * newly created menu item.
 *
 * @param label The label of the menu item.
 * @param short_label The short label of the menu item.
 * @param type The type of the menu item.
 * @return A pointer to the newly created menu item.
 */
menu_item_t* menu_create_item(const char* label, const char* short_label, menu_type_t type) {
    menu_item_t* item = malloc(sizeof(menu_item_t));
    if (!item) return NULL;

    memset(item, 0, sizeof(menu_item_t));
    item->label = label;
    item->label_short = short_label;
    item->type = type;
    return item;
}


/**
 * @brief Add a child menu item to a parent menu item.
 *
 * This function adds a child menu item to a parent menu item.  The function
 * reallocates memory for the parent menu item's children array to add the
 * new child menu item.  The function returns true if the child menu item was
 * successfully added, or false otherwise.
 *
 * @param parent The parent menu item to which to add the child menu item.
 * @param child The child menu item to add to the parent menu item.
 * @return True if the child menu item was successfully added, false otherwise.
 */
bool menu_add_child(menu_item_t* parent, const menu_item_t* child) {
    if (!parent || !child) return false;

    // Create new array with space for one more child
    const menu_item_t* const* new_children = realloc((void*)parent->children,
                                                    (parent->child_count + 1) * sizeof(menu_item_t*));
    if (!new_children) return false;

    // Cast is safe because we're modifying a non-const parent
    parent->children = new_children;
    ((const menu_item_t**)new_children)[parent->child_count] = child;  // Cast to allow array write
    parent->child_count++;
    return true;
}

/**
 * @brief Free a menu item and its children.
 *
 * This function frees a menu item and its children recursively.  The function
 * frees the memory for the menu item and its children, including the operation
 * data and condition rules.
 *
 * @param item The menu item to free.
 */
void menu_free_item(const menu_item_t* item) {
    if (!item) return;

    // Free children recursively
    if (item->children) {
        for (uint8_t i = 0; i < item->child_count; i++) {
            menu_free_item(item->children[i]);
        }
        free((void*)item->children);
    }

    // Free operation data
    if (item->operation.inputs) {
        free((void*)item->operation.inputs);
    }
    if (item->operation.confirm) {
        free((void*)item->operation.confirm);
    }
    if (item->operation.result) {
        free((void*)item->operation.result);
    }

    // Free condition rules
    if (item->conditions.rules) {
        free((void*)item->conditions.rules);
    }

    free((void*)item);
}
