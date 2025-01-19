#pragma once

#include <stdbool.h>
#include "../structure/menu_item.h"
#include "operation_types.h"

// Main operation execution
bool start_operation(const menu_item_t* item);
void execute_operation(void);

// Helper functions
operation_phase_t get_current_operation_phase(void);
bool is_operation_in_progress(void);
bool cancel_operation(bool is_error);
bool set_operation_selection(int8_t selection);
