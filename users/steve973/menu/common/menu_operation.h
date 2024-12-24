#pragma once

#include <stdbool.h>
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

typedef enum {
    PHASE_STATE_INIT,
    PHASE_STATE_AWAITING_INPUT,
    PHASE_STATE_PROCESSING,
    PHASE_STATE_COMPLETE,
    PHASE_STATE_CANCELLED
} phase_state_t;

// Operation results
typedef enum {
    OPERATION_RESULT_NONE,
    OPERATION_RESULT_SUCCESS,
    OPERATION_RESULT_CANCELLED,  // User cancelled
    OPERATION_RESULT_REJECTED,   // Validation/precondition failed
    OPERATION_RESULT_ERROR,      // Something went wrong
    OPERATION_RESULT_TIMEOUT     // Operation timed out
} operation_result_t;

typedef struct operation_context {
    operation_phase_t current_phase;  // Current phase of operation
    phase_state_t phase_state;        // State of the current phase
    const menu_item_t* item;          // All config comes from here
    int8_t choice_made;               // -1 if no choice made, otherwise index of choice
    void** phase_data;                // For phase-to-phase communication
    operation_result_t result;        // Result of the operation
} operation_context_t;

// Main entry point for operation execution
operation_result_t execute_operation(const menu_item_t* item);

// Helper functions
operation_phase_t get_current_operation_phase(void);
bool is_operation_in_progress(void);
