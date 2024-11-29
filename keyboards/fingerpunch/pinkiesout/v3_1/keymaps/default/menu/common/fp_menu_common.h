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

#include "oled_driver.h"
#include "quantum.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"
#include "keyboards/fingerpunch/src/fp_rgb_common.h"
#include "quantum/rgb_matrix/rgb_matrix.h"

#define MAX_MENU_DEPTH 5
#define ITEMS_PER_PAGE 10

typedef enum {
    MENU_TYPE_ACTION,    // Execute function
    MENU_TYPE_SUBMENU,   // Push submenu
    MENU_TYPE_DISPLAY,   // Push display screen
    MENU_TYPE_INFO       // Display info
} menu_type_t;

typedef struct menu_item {
    const char* name;
    void (*action)(void);
    menu_type_t type;
} menu_item_t;

typedef struct menu_state {
    const char* title;
    uint8_t current_index;
    const menu_item_t* items;
    uint8_t item_count;
} menu_state_t;

extern const menu_item_t MAIN_MENU[];

void init_menu_stack(void);
void set_in_menu_mode(bool in_menu_mode);
bool get_in_menu_mode(void);
void menu_stack_push(const char* title, const menu_item_t* items, uint8_t count);
void menu_stack_pop(void);
void menu_stack_home(void);
menu_state_t* menu_stack_current(void);
void display_current_menu(void);
bool process_menu_record(uint16_t keycode, keyrecord_t *record);
