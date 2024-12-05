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

#include "fp_joystick_menu.h"

const menu_item_t JOYSTICK_MENU[] = {
    {"Calibration", show_joystick_calibration_menu, MENU_TYPE_SUBMENU},
    {"Mode", show_joystick_mode_menu, MENU_TYPE_SUBMENU},
    {"Sensitivity", show_joystick_sensitivity_menu, MENU_TYPE_SUBMENU},
    {"Orientation", detect_orientation, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t JOYSTICK_MENU_SIZE = sizeof(JOYSTICK_MENU) / sizeof(menu_item_t);

void show_joystick_menu(void) {
    menu_stack_push("Joystick Menu", JOYSTICK_MENU, JOYSTICK_MENU_SIZE);
}

void detect_orientation(void) {
    // Clear and show initial instructions
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("Set JS Orientation"), false);
    oled_set_cursor(0, 1);
    oled_write_P(PSTR(""), false);
    oled_set_cursor(0, 2);
    oled_write_P(PSTR("Hold JS in physical"), false);
    oled_set_cursor(0, 3);
    oled_write_P(PSTR("UP direction..."), false);
    oled_set_cursor(0, 4);
    oled_write_P(PSTR(""), false);
    oled_render_dirty(true);
    wait_ms(1000);

    // Wait for movement or escape
    if (wait_for_js_movement(10000, 85)) {
        int8_t up_direction = calculate_direction(false);
        if (up_direction != JS_NEUTRAL) {
            joystick_config.up_orientation = up_direction;
            fp_kb_config_user.js_config = joystick_config;
            fp_kb_config_save();

            // Show confirmation
            oled_set_cursor(0, 5);
            oled_write_P(PSTR("Orientation set!"), false);
            oled_render_dirty(true);
            wait_ms(1000);
        }
    }

    // Return to menu
    oled_clear();
    display_current_menu();
}
