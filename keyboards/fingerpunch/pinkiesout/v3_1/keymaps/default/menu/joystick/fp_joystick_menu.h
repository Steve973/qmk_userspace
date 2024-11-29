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
extern const menu_item_t JOYSTICK_MENU[];
extern const menu_item_t JOYSTICK_CALIBRATION_MENU[];
extern const menu_item_t JOYSTICK_MODE_MENU[];
extern const menu_item_t JOYSTICK_SENSITIVITY_MENU[];

// Joystick menu functions
void show_joystick_menu(void);
void detect_orientation(void);

// Joystick calibration functions
void show_joystick_calibration_menu(void);
void calibrate_joystick_neutral(void);
void calibrate_joystick_range(void);

// Joystick mode functions
void show_joystick_mode_menu(void);
void set_arrow_mode(void);
void set_wasd_mode(void);
void set_analog_mode(void);
void set_mouse_mode(void);

// Joystick sensitivity functions
void show_joystick_sensitivity_menu(void);
void set_x_axis_sensitivity(void);
void set_y_axis_sensitivity(void);
