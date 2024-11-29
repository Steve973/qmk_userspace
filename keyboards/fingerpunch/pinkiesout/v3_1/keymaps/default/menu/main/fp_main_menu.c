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

#include "fp_main_menu.h"

const menu_item_t MAIN_MENU[] = {
    {"Joystick", show_joystick_menu, MENU_TYPE_SUBMENU},
    {"RGB", show_rgb_menu, MENU_TYPE_SUBMENU},
    {"System", show_system_menu, MENU_TYPE_SUBMENU},
    {"KB Stats", show_kb_stats, MENU_TYPE_DISPLAY},
    {"Exit", exit_menu, MENU_TYPE_ACTION}
};

const size_t MAIN_MENU_SIZE = sizeof(MAIN_MENU) / sizeof(menu_item_t);

void exit_menu(void) {
    set_in_menu_mode(false);
    oled_clear();
}
