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

#include "fp_rgb_menu.h"

const menu_item_t RGB_SPEED_MENU[] = {
    {"Step Speed", step_speed, MENU_TYPE_ACTION},
    {"Step Speed Reverse", step_speed_reverse, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t RGB_SPEED_MENU_SIZE = sizeof(RGB_SPEED_MENU) / sizeof(menu_item_t);

void show_rgb_speed_menu(void) {
    menu_stack_push("RGB Speed", RGB_SPEED_MENU, RGB_SPEED_MENU_SIZE);
}

void step_speed(void) {
    // Set the RGB speed
}

void step_speed_reverse(void) {
    // Set the RGB speed
}
