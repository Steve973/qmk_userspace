#pragma once

#include "../../common/menu_operation.h"

typedef enum {
    PHASE_RESULT_CONTINUE,  // Stay in current state
    PHASE_RESULT_ADVANCE,   // Move to next state
    PHASE_RESULT_COMPLETE,  // Skip to complete
    PHASE_RESULT_CANCEL,    // Cancel operation
    PHASE_RESULT_ERROR      // Error occurred
} phase_result_t;

// Operation phase management
void advance_operation_phase(operation_context_t* context);
void set_operation_phase(operation_context_t* context, operation_phase_t phase);
void complete_operation(operation_context_t* context);  // Sets to OPERATION_PHASE_COMPLETE

// Phase state management
void advance_phase_state(operation_context_t* context);
void set_phase_state(operation_context_t* context, phase_state_t state);
void complete_phase_state(operation_context_t* context);  // Sets to PHASE_STATE_COMPLETE

// Initialization
void state_manager_init(operation_context_t* context);
