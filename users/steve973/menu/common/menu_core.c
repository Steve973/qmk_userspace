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

    // Display state
    bool show_shortcuts;             // Show keyboard shortcuts

    // Menu history
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
        rgb_matrix_reload_from_eeprom();
    }
    #endif
}

/* Public API Implementation */
bool menu_init(void) {
    dprintf("Initializing menu\n");
    menu_state = (menu_state_t) {
        .current = menu_root,
        .selected_index = 0,
        .last_activity = timer_read32(),
        .timeout_ms = DEFAULT_TIMEOUT_MS,
        .show_shortcuts = false,
        .history = { .depth = 0 },
        .operation = {
            .in_progress = false,
            .item = NULL,
            .input_value = NULL,
            .previous_value = NULL
        },
    };

    return true;
}

bool is_menu_active(void) {
    return menu_active;
}

void menu_exit(void) {
    set_menu_active(false);
}

void set_menu_active(bool active) {
    if (active && !menu_active) {
        // Add debug prints to track execution
        dprintf("Setting menu active, creating root screen\n");
        menu_active = true;
        menu_init();

        // Create and push new submenu screen
        screen_content_t* screen = create_menu_screen(menu_state.current);
        push_screen((managed_screen_t){
            .owner = MENU_OWNER,
            .is_custom = false,
            .display.content = screen,
            .refresh_interval_ms = 0
        });

        update_menu_activity();
        menu_timeout_token = timeout_indicator_create(menu_state.timeout_ms, &menu_exit);
    } else if (!active && menu_active) {
        dprintf("Deactivating menu\n");
        timeout_indicator_cancel(menu_timeout_token);
        dprintf("Clearing keyboard...\n");
        // Remove the main menu screen
        pop_screen(MENU_OWNER);
        clear_keyboard();
        dprintf("Setting menu active to false...\n");
        menu_active = false;
        clear_display();
    }
    dprintf("Setting menu mode lighting 'active' to %d\n", active);
    wait_ms(100);
    set_menu_mode_lighting(active);
    wait_ms(100);
    dprintf("Menu active state set (complete)\n");
    wait_ms(100);
}

bool menu_enter(void) {
    const menu_item_t* current = menu_state.current;
    if (!current) return false;

    // If we're at a submenu, navigate into it
    if (current->type == MENU_TYPE_SUBMENU) {
        if (!current->children || current->child_count == 0) {
            return false;  // Empty submenu
        }

        // Save current position in history
        if (menu_state.history.depth < MAX_MENU_DEPTH) {
            menu_state.history.items[menu_state.history.depth++] = current;
        }

        // Create and push new submenu screen
        screen_content_t* screen = create_menu_screen(current->children[menu_state.selected_index]);
        push_screen((managed_screen_t){
            .owner = MENU_OWNER,
            .is_custom = false,
            .display.content = screen,
            .refresh_interval_ms = 0
        });

        // Move to first item in submenu
        menu_state.current = current->children[menu_state.selected_index];
        menu_state.selected_index = 0;
        return true;
    }

    // If it's an action, execute it
    else if (current->type == MENU_TYPE_ACTION && current->operation.action) {
        execute_operation(current);
        return true;
    }

    return false;
}

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

static void handle_navigation(uint16_t keycode, nav_context_t context) {
    dprintf("Navigation: keycode=%u, context=%d\n", keycode, context);

    uint8_t item_count;
    uint8_t* selected_index;

    // Set up context-specific vars
    if (context == NAV_CONTEXT_MENU) {
        if (!menu_state.current) {
            dprintf("Error: menu_state.current is NULL\n");
            return;
        }
        item_count = menu_state.current->child_count;
        selected_index = &menu_state.selected_index;
        dprintf("Menu navigation: items=%d, current_index=%d\n",
                item_count, *selected_index);
    } else {
        if (!menu_state.operation.item) {
            dprintf("Error: operation item is NULL\n");
            return;
        }
        item_count = menu_state.operation.item->operation.input_count;
        selected_index = &menu_state.selected_index;
        dprintf("Operation navigation: items=%d, current_index=%d\n",
                item_count, *selected_index);
    }

    switch (keycode) {
        case KC_W:
        case KC_UP:
            dprintf("Up navigation\n");
            if (*selected_index > 0) {
                (*selected_index)--;
                dprintf("Selected index decreased to %d\n", *selected_index);
            } else {
                *selected_index = item_count - 1;
                dprintf("Wrapped to bottom: %d\n", *selected_index);
            }
            break;

        case KC_S:
        case KC_DOWN:
            dprintf("Down navigation\n");
            if (*selected_index < item_count - 1) {
                (*selected_index)++;
                dprintf("Selected index increased to %d\n", *selected_index);
            } else {
                *selected_index = 0;
                dprintf("Wrapped to top\n");
            }
            break;

        case KC_D:
        case KC_ENTER:
        case KC_RIGHT:
            dprintf("Enter/Right navigation\n");
            if (context == NAV_CONTEXT_MENU) {
                dprintf("Attempting menu enter\n");
                menu_enter();
            }
            break;

        case KC_A:
        case KC_ESC:
        case KC_LEFT:
            dprintf("Exit/Left navigation\n");
            if (context == NAV_CONTEXT_MENU) {
                dprintf("Attempting menu back\n");
                if (menu_state.history.depth == 0) {
                    dprintf("At root menu, exiting\n");
                    set_menu_active(false);
                } else {
                    dprintf("Going back one level\n");
                    menu_back();
                }
            }
            dprintf("Exit/Left navigation complete\n");
            break;

        default:
            dprintf("Checking shortcuts for keycode: %u\n", keycode);
            if (context == NAV_CONTEXT_MENU &&
                menu_state.show_shortcuts &&
                menu_state.current->children) {
                // Handle shortcuts
                for (uint8_t i = 0; i < menu_state.current->child_count; i++) {
                    const menu_item_t* item = menu_state.current->children[i];
                    if (item->shortcut && keycode == item->shortcut[0]) {
                        dprintf("Shortcut found for item %d\n", i);
                        menu_state.selected_index = i;
                        menu_enter();
                    }
                }
            }
            break;
    }
    dprintf("Navigation handling complete\n");
}

/* Input Processing */
bool process_menu_record(uint16_t keycode, keyrecord_t *record) {
    dprintf("Menu record: keycode=%u, pressed=%d\n", keycode, record->event.pressed);
    if (!record->event.pressed) return false;

    const menu_item_t* current = menu_state.current;
    if (!current) return false;

    dprintf("Current menu item: %s\n", current->label);

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

    dprintf("Navigation context: %d\n", context);

    handle_navigation(keycode, context);
    update_menu_activity();

    dprintf("Menu record processing complete\n");
    return false;
}

/* Memory Management */
menu_item_t* menu_create_item(const char* label, const char* short_label, menu_type_t type) {
    menu_item_t* item = malloc(sizeof(menu_item_t));
    if (!item) return NULL;

    memset(item, 0, sizeof(menu_item_t));
    item->label = label;
    item->label_short = short_label;
    item->type = type;
    return item;
}

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
