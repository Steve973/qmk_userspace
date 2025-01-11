#pragma once

#include "../../core/operation/operation_types.h"

// Operation phase management
void advance_operation_phase(operation_context_t* context);
void set_operation_phase(operation_context_t* context, operation_phase_t phase);
void complete_operation(operation_context_t* context);  // Sets to OPERATION_PHASE_COMPLETE

// Phase state management
void advance_phase_state(operation_context_t* context);
void set_phase_state(operation_context_t* context, phase_state_t state);
void complete_phase_state(operation_context_t* context);  // Sets to PHASE_STATE_COMPLETE

// Initialization
void operation_lifecycle_manager_init(operation_context_t* context);
