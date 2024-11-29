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

const menu_item_t RGB_VALUE_MENU[] = {
    {"Step Value", step_value, MENU_TYPE_ACTION},
    {"Step Value Reverse", step_value_reverse, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t RGB_VALUE_MENU_SIZE = sizeof(RGB_VALUE_MENU) / sizeof(menu_item_t);

void show_rgb_value_menu(void) {
    menu_stack_push("RGB Value", RGB_VALUE_MENU, RGB_VALUE_MENU_SIZE);
}

void step_value(void) {
    fp_rgblight_increase_val();
}

void step_value_reverse(void) {
    fp_rgblight_decrease_val();
}
