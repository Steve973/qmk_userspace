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
    // Detect the orientation of the joystick
}

