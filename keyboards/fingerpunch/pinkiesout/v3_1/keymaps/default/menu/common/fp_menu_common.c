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

#include "fp_menu_common.h"

static struct {
    menu_state_t states[MAX_MENU_DEPTH];
    uint8_t depth;
} menu_stack;

extern const size_t MAIN_MENU_SIZE;

static bool in_menu_mode = false;
static deferred_token menu_timeout_token = INVALID_DEFERRED_TOKEN;
uint16_t menu_timeout_ms = 30000;
uint32_t last_menu_activity = 0;

/**
 * @brief Update the menu activity timer.
 * 
 * Updates the menu activity timer to the current time.
 */
static void update_menu_activity(void) {
    last_menu_activity = timer_read32();
}

/**
 * @brief Check the menu timeout.
 * 
 * Checks the menu timeout and returns the time until the next check.
 * If the menu has been inactive for the timeout period, this will
 * exit menu mode.
 * 
 * @param trigger_time The time the timer was triggered.
 * @param cb_arg The callback argument.
 * @return The time until the next check.
 */
uint32_t check_menu_timeout(uint32_t trigger_time, void* cb_arg) {
    uint32_t elapsed = timer_read32() - last_menu_activity;
    if (elapsed >= menu_timeout_ms) {
        set_in_menu_mode(false);
        return 0;
    }

    // Draw decay line using fixed point math
    uint8_t height = OLED_DISPLAY_HEIGHT;
    // Convert to 8.8 fixed point: (elapsed << 8) / menu_timeout_ms
    uint16_t progress = (elapsed << 8) / menu_timeout_ms;
    // 256 - progress gives inverted percentage in 8.8
    uint16_t remaining_fixed = 256 - progress;
    // Scale height by remaining (in 8.8), then shift back
    uint8_t remaining = (height * remaining_fixed) >> 8;
    
    // Draw vertical line at rightmost column, and as the remaining
    // time decreases, bottom pixels turn off first.
    for (uint8_t i = 0; i < height; i++) {
        oled_write_pixel(OLED_DISPLAY_WIDTH-1, i, i < remaining);
    }

    // Check again in 100 ms
    return 100;
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

/**
 * @brief Toggles between menu mode and normal mode.
 * 
 * When invoked with "true", this function sets the menu mode flag and displays
 * the main menu, and sets the keyboard lighting to indicate menu mode.  When
 * invoked with "false", this function clears the menu stack and exits menu mode.
 * Upon exiting menu mode, the keyboard lighting is restored to the state it was
 * in before entering menu mode.
 * 
 * @param menu_mode True to enter menu mode, false to exit.
 */
void set_in_menu_mode(bool menu_mode) {
    if (menu_mode) {
        in_menu_mode = menu_mode;
        menu_stack_home();
        update_menu_activity();
        menu_timeout_token = defer_exec(100, check_menu_timeout, NULL);
    } else {
        cancel_deferred_exec(menu_timeout_token);
        clear_keyboard();
        init_menu_stack();
        in_menu_mode = menu_mode;
    }
    set_menu_mode_lighting(menu_mode);
}

/**
 * Get the menu mode flag.
 */
bool get_in_menu_mode(void) {
    return in_menu_mode;
}

/**
 * @brief Push a new menu onto the stack.
 * 
 * Pushes a new menu onto the stack, if the stack is not full.  This will clear the OLED
 * and display the new menu.
 * 
 * @param title The title of the menu.
 * @param items The items in the menu.
 * @param count The number of items in the menu.
 */
void menu_stack_push(const char* title, const menu_item_t* items, uint8_t count) {
    if (menu_stack.depth < MAX_MENU_DEPTH) {
        oled_clear();
        menu_stack.states[menu_stack.depth] = (menu_state_t){
            .title = title,
            .current_index = 0,
            .items = items,
            .item_count = count
        };
        menu_stack.depth++;
    }
    display_current_menu();
}

/**
 * @brief Pop the current menu off the stack.
 * 
 * Pops the current menu off the stack, if the stack is not empty.  This will clear the
 * OLED to allow the previous menu (that is not on the top of the menu stack) to be
 * displayed.  If the stack is empty after popping, this will exit menu mode.
 */
void menu_stack_pop(void) {
    if (menu_stack.depth > 1) {
        menu_stack.depth--;
    } else {
        set_in_menu_mode(false);
    }
    oled_clear();
}

/**
 * @brief Return to the main menu.
 * 
 * Clears the menu stack, then pushes the main menu onto the stack to allow it to be
 * displayed.
 */
void menu_stack_home(void) {
    init_menu_stack();
    menu_stack_push("Main Menu", MAIN_MENU, MAIN_MENU_SIZE);
}

/**
 * @brief Get the current menu state.
 * 
 * Gets the current menu on the top of the stack, or the "current" menu.
 */
menu_state_t* menu_stack_current(void) {
    return &menu_stack.states[menu_stack.depth - 1];
}

/**
 * @brief Initialize the menu stack.
 * 
 * Initialize the menu stack by setting it to empty.  This will set the depth to 0 and
 * clear all menu states.
 */
void init_menu_stack(void) {
    menu_stack.depth = 0;
    for (uint8_t i = 0; i < MAX_MENU_DEPTH; i++) {
        menu_stack.states[i] = (menu_state_t){
            .current_index = 0,
            .items = NULL,
            .item_count = 0
        };
    }
}

/**
 * @brief Display the current menu.
 * 
 * Display the current menu on the oled.  This will display the first "ITEMS_PER_PAGE" items,
 * and if there are more items, it will display "Next Page" and "Prev Page" at the bottom,
 * depending on what is being displayed.
 */
void display_current_menu(void) {
    menu_state_t* current = menu_stack_current();
    if (!current || !current->items) return;

    // Display menu title with blank line after
    oled_set_cursor(0, 0);
    oled_write_P(current->title, false);
    oled_set_cursor(0, 1);
    oled_write_P(PSTR(""), false);  // blank line

    // Calculate paging
    const uint8_t total_pages = (current->item_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    const uint8_t current_page = current->current_index / ITEMS_PER_PAGE;
    const uint8_t page_start = current_page * ITEMS_PER_PAGE;
    const uint8_t items_on_page = MIN(ITEMS_PER_PAGE, current->item_count - page_start);

    // Show menu items starting below title and blank line
    for (uint8_t i = 0; i < items_on_page; i++) {
        const uint8_t item_index = page_start + i;
        oled_set_cursor(0, i + 2);  // +2 for title and blank line
        oled_write_P(current->items[item_index].name, 
                     (item_index == current->current_index));
    }

    // Navigation hints
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

/**
 * @brief Process a keypress in menu mode.
 * 
 * Process a keypress in menu mode.  This will handle navigation and executing actions.
 * 
 * @param keycode The keycode of the key that was pressed.
 * @param record The keyrecord of the key that was pressed.
 * @return True if the keypress should be processed by the main keymap, false otherwise.
 */
bool process_menu_record(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        menu_state_t* current = menu_stack_current();
        switch (keycode) {
            case KC_W:
            case KC_UP:
                if (current->current_index > 0) {
                    current->current_index--;
                }
                break;
            case KC_S:
            case KC_DOWN:
                if (current->current_index < current->item_count - 1) {
                    current->current_index++;
                }
                break;
            case KC_D:
            case KC_ENTER:
            case KC_RIGHT:
                const menu_item_t* item = &current->items[current->current_index];
                if (item->action) {
                    item->action();
                }
                break;
            case KC_A:
            case KC_ESC:
            case KC_LEFT:
                menu_stack_pop();
                break;
        }
        update_menu_activity();
    }
    return false;
}
