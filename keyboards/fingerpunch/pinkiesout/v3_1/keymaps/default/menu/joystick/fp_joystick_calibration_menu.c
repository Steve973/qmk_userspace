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

const menu_item_t JOYSTICK_CALIBRATION_MENU[] = {
    {"Neutral", calibrate_joystick_neutral, MENU_TYPE_ACTION},
    {"Range", calibrate_joystick_range, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t JOYSTICK_CALIBRATION_MENU_SIZE = sizeof(JOYSTICK_CALIBRATION_MENU) / sizeof(menu_item_t);

void show_joystick_calibration_menu(void) {
    menu_stack_push("JS Calibration", JOYSTICK_CALIBRATION_MENU, JOYSTICK_CALIBRATION_MENU_SIZE);
}

void calibrate_joystick_neutral(void) {
    // Clear and show initial instructions
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("JS Neutral Cal"), false);
    oled_set_cursor(0, 1);
    oled_write_P(PSTR(""), false);
    oled_set_cursor(0, 2);
    oled_write_P(PSTR("Do not touch JS"), false);
    oled_set_cursor(0, 3);
    oled_write_P(PSTR("while calibrating..."), false);
    oled_set_cursor(0, 4);
    oled_write_P(PSTR(""), false);
    oled_render_dirty(true);
    wait_ms(1000);

    // Run calibration
    calibrate_neutral_values(false);

    // Show completion message
    oled_set_cursor(0, 5);
    oled_write_P(PSTR("Calibration complete!"), false);
    oled_render_dirty(true);
    
    // Show confirmation for 2 seconds before returning
    wait_ms(2000);
    oled_clear();
    display_current_menu();
}

void calibrate_joystick_range(void) {
    // Clear and show initial instructions
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("JS Range Cal"), false);
    oled_set_cursor(0, 1);
    oled_write_P(PSTR(""), false);
    oled_set_cursor(0, 2);
    oled_write_P(PSTR("Rotate the JS"), false);
    oled_set_cursor(0, 3);
    oled_write_P(PSTR("at full range..."), false);
    oled_set_cursor(0, 4);
    oled_write_P(PSTR(""), false);
    oled_render_dirty(true);
    wait_ms(1000);

    // Run calibration
    calibrate_range(false);

    // Show completion message
    oled_set_cursor(0, 5);
    oled_write_P(PSTR("Calibration complete!"), false);
    oled_render_dirty(true);
    
    // Show confirmation for 2 seconds before returning
    wait_ms(2000);
    oled_clear();
    display_current_menu();
}
