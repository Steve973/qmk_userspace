#pragma once

#include <stdint.h>
#include "../structure/menu_item.h"

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

typedef enum {
    PHASE_STATE_NONE,
    PHASE_STATE_INIT,
    PHASE_STATE_AWAITING_INPUT,
    PHASE_STATE_PROCESSING,
    PHASE_STATE_COMPLETE,
    PHASE_STATE_CANCELLED
} phase_state_t;

typedef enum {
    PHASE_RESULT_CONTINUE,  // Stay in current state
    PHASE_RESULT_ADVANCE,   // Move to next state
    PHASE_RESULT_COMPLETE,  // Skip to complete
    PHASE_RESULT_CANCEL,    // Cancel operation
    PHASE_RESULT_ERROR      // Error occurred
} phase_result_t;

typedef struct operation_context {
    operation_phase_t current_phase;  // Current phase of operation
    phase_state_t phase_state;        // State of the current phase
    const menu_item_t* item;          // All config comes from here
    int8_t choice_made;               // -1 if no choice made, otherwise index of choice
    void** phase_data;                // For phase-to-phase communication
    operation_result_t result;        // Result of the operation
} operation_context_t;

typedef operation_result_t (*action_func_t)(operation_result_t prev_result, void** input_values);

typedef struct {
    const char* name;
    action_func_t func;
} action_lookup_entry_t;
