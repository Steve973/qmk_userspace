#pragma once

#include <stdbool.h>
#include "../structure/menu_item.h"

// Core navigation functions
void menu_home(void);
bool menu_enter(void);
bool menu_back(void);
bool menu_pop(void);

// Navigation queries
bool can_navigate_back(void);

// Navigation state
const menu_item_t* get_current_menu(void);
