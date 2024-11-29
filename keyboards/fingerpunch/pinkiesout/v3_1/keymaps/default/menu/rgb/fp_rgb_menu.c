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

const menu_item_t RGB_MENU[] = {
    {"Mode", show_rgb_mode_menu, MENU_TYPE_SUBMENU},
    {"Hue", show_rgb_hue_menu, MENU_TYPE_SUBMENU},
    {"Saturation", show_rgb_saturation_menu, MENU_TYPE_SUBMENU},
    {"Value", show_rgb_value_menu, MENU_TYPE_SUBMENU},
    {"Speed", show_rgb_speed_menu, MENU_TYPE_SUBMENU},
    {"Toggle RGB", toggle_rgb, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t RGB_MENU_SIZE = sizeof(RGB_MENU) / sizeof(menu_item_t);

void show_rgb_menu(void) {
    menu_stack_push("RGB Menu", RGB_MENU, RGB_MENU_SIZE);
}

void toggle_rgb(void) {
    rgb_matrix_toggle_noeeprom();
}
