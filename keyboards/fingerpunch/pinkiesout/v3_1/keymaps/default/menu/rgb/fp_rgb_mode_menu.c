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

#include "menu/rgb/fp_rgb_menu.h"

const menu_item_t RGB_MODE_MENU[] = {
    {"Step Mode", step_mode, MENU_TYPE_ACTION},
    {"Step Mode Reverse", step_mode_reverse, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t RGB_MODE_MENU_SIZE = sizeof(RGB_MODE_MENU) / sizeof(menu_item_t);

void show_rgb_mode_menu(void) {
    menu_stack_push("RGB Mode", RGB_MODE_MENU, RGB_MODE_MENU_SIZE);
}

void step_mode(void) {
    fp_rgblight_step();
}

void step_mode_reverse(void) {
    fp_rgblight_step_reverse();
}
