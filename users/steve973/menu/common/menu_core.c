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
#include "oled/oled_driver.h"
#include "keycodes.h"
#include "timer.h"
#include "src/fp_rgb_common.h"
#include "oled/timeout_indicator/timeout_indicator.h"
#include "menu/common/menu_operation.h"
#include "menu/common/menu_core.h"

#define ITEMS_PER_PAGE 10
#define DEFAULT_TIMEOUT_MS 30000

/* Menu State Management - Internal */
typedef struct menu_state {
    const menu_item_t* current;      // Current menu being displayed
    uint8_t selected_index;          // Currently selected item
    uint32_t last_activity;          // For timeout tracking
    uint32_t timeout_ms;             // Configurable timeout

    // Display state
    bool show_shortcuts;             // Show keyboard shortcuts
    uint8_t selection_style;         // Highlight/arrow/star

    // History for back navigation
    struct {
        const menu_item_t* items[MAX_MENU_DEPTH];
        uint8_t indices[MAX_MENU_DEPTH];
        uint8_t depth;
    } history;

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
    menu_state = (menu_state_t) {
        .current = menu_root,
        .selected_index = 0,
        .last_activity = timer_read32(),
        .timeout_ms = DEFAULT_TIMEOUT_MS,
        .show_shortcuts = false,
        .selection_style = SELECTION_HIGHLIGHT,
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

void set_menu_active(bool active) {
    menu_active = active;
    if (active) {
        update_menu_activity();
        oled_clear();
        menu_timeout_token = timeout_indicator_create(menu_state.timeout_ms, &menu_exit);
    } else {
        timeout_indicator_cancel(menu_timeout_token);
        clear_keyboard();
        oled_clear();
    }
    set_menu_mode_lighting(active);
}

void menu_exit(void) {
    set_menu_active(false);
}

bool menu_enter(void) {
    const menu_item_t* current = menu_state.current;
    if (!current) return false;

    if (current->type == MENU_TYPE_ACTION) {
        if (current->operation.action) {
            // TODO - What to do with return value?
            execute_operation(current);
        }
        return false;
    }

    if (current->type == MENU_TYPE_SUBMENU && current->children && current->child_count > 0) {
        if (menu_state.history.depth < MAX_MENU_DEPTH) {
            menu_state.history.items[menu_state.history.depth] = current;
            menu_state.history.indices[menu_state.history.depth] = menu_state.selected_index;
            menu_state.history.depth++;
        }

        menu_state.current = current->children[menu_state.selected_index];
        menu_state.selected_index = 0;
        oled_clear();
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
        menu_state.history.depth--;
        menu_state.current = menu_state.history.items[menu_state.history.depth];
        menu_state.selected_index = menu_state.history.indices[menu_state.history.depth];
        oled_clear();
       return true;
    }

    return false;
}

/* Display Implementation */
#ifdef OLED_ENABLE
static void display_current_menu_oled(void) {
    const menu_item_t* current = menu_state.current;
    if (!current || !current->children) return;

    oled_set_cursor(0, 0);
    oled_write_P(current->label, false);
    oled_set_cursor(0, 1);
    oled_write_P(PSTR(""), false);

    const uint8_t total_pages = (current->child_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    const uint8_t current_page = menu_state.selected_index / ITEMS_PER_PAGE;
    const uint8_t page_start = current_page * ITEMS_PER_PAGE;
    const uint8_t items_on_page = MIN(ITEMS_PER_PAGE, current->child_count - page_start);

    for (uint8_t i = 0; i < items_on_page; i++) {
        const uint8_t item_index = page_start + i;
        oled_set_cursor(0, i + 2);

        const menu_item_t* item = current->children[item_index];
        const char* display_text = menu_state.show_shortcuts && item->shortcut ?
            item->label_short : item->label;

        oled_write_P(display_text, (item_index == menu_state.selected_index));
    }

    if (total_pages > 1) {
        if (current_page > 0) {
            oled_set_cursor(0, ITEMS_PER_PAGE + 2);
            oled_write_P(PSTR("< Prev"), false);
        }
        if (current_page < total_pages - 1) {
            oled_set_cursor(10, ITEMS_PER_PAGE + 2);
            oled_write_P(PSTR("Next >"), false);
        }
    }

    oled_render_dirty(true);
}
#endif

void display_current_menu(void) {
    if (!is_operation_in_progress()) {
        display_current_menu_oled();
    }
}

/* Input Processing */
bool process_menu_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return false;

    const menu_item_t* current = menu_state.current;
    if (!current) return false;

    if (is_operation_in_progress()) {
        // TODO: Call the appropriate input handling function
        //       for the current operation lifecycle phase
        //       and return false so the key is not processed
        //       anywhere else
        return false;
    }

    switch (keycode) {
        case KC_W:
        case KC_UP:
            if (menu_state.selected_index > 0) {
                menu_state.selected_index--;
            }
            break;

        case KC_S:
        case KC_DOWN:
            if (menu_state.selected_index < current->child_count - 1) {
                menu_state.selected_index++;
            }
            break;

        case KC_D:
        case KC_ENTER:
        case KC_RIGHT:
            menu_enter();
            break;

        case KC_A:
        case KC_ESC:
        case KC_LEFT:
            if (menu_state.history.depth == 0) {
                set_menu_active(false);
            } else {
                menu_back();
            }
            break;

        default:
            // Handle shortcuts
            if (menu_state.show_shortcuts && current->children) {
                for (uint8_t i = 0; i < current->child_count; i++) {
                    const menu_item_t* item = current->children[i];
                    if (item->shortcut && keycode == item->shortcut[0]) {
                        menu_state.selected_index = i;
                        menu_enter();
                    }
                }
            }
            break;
    }

    update_menu_activity();
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
