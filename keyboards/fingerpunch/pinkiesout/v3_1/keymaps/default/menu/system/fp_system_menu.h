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
#include "quantum/audio/audio.h"
#include "version.h"

#define MAX_INFO_STR 32

// Menu definitions
extern const menu_item_t SYSTEM_MENU[];
extern const menu_item_t DEBUG_INFO_DISPLAY[];
extern char debug_info_strs[7][MAX_INFO_STR];

// System menu Functions
void show_system_menu(void);
void reset_eeprom(void);
void enter_bootloader(void);
void show_debug_info(void);
void test_piezo(void);
