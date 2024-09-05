#pragma once

#include <stdint.h>
#include "action.h"

// Menu mode activation/deactivation
void enter_menu_mode(void);
void exit_menu_mode(void);

// Core input processing
bool process_menu_record(uint16_t keycode, keyrecord_t *record);

// Display-related functions needed by other components
int8_t get_current_highlight_index(void);

// Global configuration
void set_menu_timeout(uint32_t timeout_ms);
void set_shortcuts_visible(bool visible);

void update_menu_activity(void);
