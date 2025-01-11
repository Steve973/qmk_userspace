#pragma once

#include "display_manager/display_manager.h"
#include "../core/structure/menu_item.h"
#include "../core/operation/operation_types.h"

#define MENU_OWNER "menu"

// Main function to create screen content from a menu item
screen_push_status_t create_menu_screen(const menu_item_t* menu_item, int8_t (*get_highlight_index)(void), const char* owner);

// For operation phases
screen_push_status_t create_operation_screen(const menu_item_t* item, operation_phase_t phase, const char* owner);

// Function to remove a menu screen when done
screen_pop_status_t remove_menu_screen(const char* owner);
