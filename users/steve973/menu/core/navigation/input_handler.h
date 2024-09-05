#pragma once

#include <stdint.h>
#include "action.h"

// Core input handling
void handle_menu_input(uint16_t keycode, keyrecord_t* record);

// Context-specific handlers
void handle_menu_navigation_input(uint16_t keycode);
void handle_operation_input(uint16_t keycode);
