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

#pragma once

#include "menu/common/fp_menu_common.h"

// Menu declaration
extern const menu_item_t RGB_MENU[];
extern const menu_item_t RGB_MODE_MENU[];
extern const menu_item_t RGB_HUE_MENU[];
extern const menu_item_t RGB_SATURATION_MENU[];
extern const menu_item_t RGB_VALUE_MENU[];
extern const menu_item_t RGB_SPEED_MENU[];

// RGB Menu functions
void show_rgb_menu(void);
void toggle_rgb(void);

// RGB Mode Menu functions
void show_rgb_mode_menu(void);
void step_mode(void);
void step_mode_reverse(void);

// RGB Hue Menu functions
void show_rgb_hue_menu(void);
void step_hue(void);
void step_hue_reverse(void);

// RGB Saturation Menu functions
void show_rgb_saturation_menu(void);
void step_saturation(void);
void step_saturation_reverse(void);

// RGB Value Menu functions
void show_rgb_value_menu(void);
void step_value(void);
void step_value_reverse(void);

// RGB Speed Menu functions
void show_rgb_speed_menu(void);
void step_speed(void);
void step_speed_reverse(void);
