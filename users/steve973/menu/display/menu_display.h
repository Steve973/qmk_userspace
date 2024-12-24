#pragma once

#include "display_manager/display_manager.h"
#include "../common/menu_core.h"
#include "../common/menu_operation.h"

// Main function to create screen content from a menu item
screen_content_t* create_menu_screen(const menu_item_t* menu_item);

// For operation phases
screen_content_t* create_operation_screen(const menu_item_t* item, operation_phase_t phase);

// Function to free a menu screen when done
void free_menu_screen(screen_content_t* screen);
