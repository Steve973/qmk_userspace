#pragma once

#include <stdint.h>
#include "action.h"

// Core input handling
bool handle_menu_input(uint16_t keycode, keyrecord_t* record);

// Context-specific handlers
bool handle_menu_navigation_input(uint16_t keycode);
bool handle_operation_input(uint16_t keycode);
