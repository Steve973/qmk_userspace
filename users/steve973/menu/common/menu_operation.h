#pragma once

#include <stdint.h>
#include "menu_core.h"

// Operation phases
typedef enum {
    OPERATION_PHASE_NONE,
    OPERATION_PHASE_PRECONDITION,
    OPERATION_PHASE_INPUT,
    OPERATION_PHASE_CONFIRMATION,
    OPERATION_PHASE_ACTION,
    OPERATION_PHASE_RESULT,
    OPERATION_PHASE_POSTCONDITION,
    OPERATION_PHASE_COMPLETE
} operation_phase_t;

// Operation results
typedef enum {
    OPERATION_RESULT_NONE,
    OPERATION_RESULT_SUCCESS,
    OPERATION_RESULT_CANCELLED,  // User cancelled
    OPERATION_RESULT_REJECTED,   // Validation/precondition failed
    OPERATION_RESULT_ERROR,      // Something went wrong
    OPERATION_RESULT_TIMEOUT     // Operation timed out
} operation_result_t;

// Main entry point for operation execution
operation_result_t execute_operation(const menu_item_t* item);

// Helper functions
operation_phase_t get_current_operation_phase(void);
bool is_operation_in_progress(void);
uint8_t get_operation_input_count(void);
