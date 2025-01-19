#include "quantum/logging/debug.h"
#include "operation_lifecycle_manager.h"
#include "../../core/operation/operation_types.h"

/**
 * @brief Set the operation phase to a specific phase.
 *
 * This function will set the operation phase to the specified phase.  This
 * function is useful for setting the operation phase to a specific phase
 * without advancing through the operation lifecycle.
 *
 * When setting a new phase, the phase state will be reset to PHASE_STATE_INIT
 * to prepare for the new phase.
 *
 * @param context The operation context to set the phase for.
 * @param phase The operation phase to set.
 */
void set_operation_phase(operation_context_t* context, operation_phase_t phase) {
    context->current_phase = phase;
    // Reset phase state when setting new phase
    context->phase_state = PHASE_STATE_INIT;
    context->choice_made = -1;
    dprintf("Set operation phase: %d\n", phase);
}

/**
 * @brief Initialize the operation state manager.
 *
 * This function should be called when starting a new operation to reset the
 * operation context to the initial state to start a new action operation
 * lifecycle.
 *
 * @param context The operation context to initialize.
 */
void operation_lifecycle_manager_init(operation_context_t* context) {
    set_operation_phase(context, OPERATION_PHASE_PRECONDITION);
}

/**
 * @brief Advance the operation phase to the next phase.
 *
 * This function will advance the operation phase to the next phase in the
 * operation lifecycle.  If the current phase is OPERATION_PHASE_COMPLETE, this
 * function will not advance the phase, since that is the terminal state.
 *
 * When advancing to a new phase, the phase state will be reset to
 * PHASE_STATE_INIT to prepare for the new phase.
 *
 * @param context The operation context to advance the phase for.
 */
void advance_operation_phase(operation_context_t* context) {
    switch(context->current_phase) {
        case OPERATION_PHASE_NONE:
            set_operation_phase(context, OPERATION_PHASE_PRECONDITION);
            break;
        case OPERATION_PHASE_PRECONDITION:
            set_operation_phase(context, OPERATION_PHASE_INPUT);
            break;
        case OPERATION_PHASE_INPUT:
            set_operation_phase(context, OPERATION_PHASE_CONFIRMATION);
            break;
        case OPERATION_PHASE_CONFIRMATION:
            set_operation_phase(context, OPERATION_PHASE_ACTION);
            break;
        case OPERATION_PHASE_ACTION:
            set_operation_phase(context, OPERATION_PHASE_RESULT);
            break;
        case OPERATION_PHASE_RESULT:
            set_operation_phase(context, OPERATION_PHASE_POSTCONDITION);
            break;
        case OPERATION_PHASE_POSTCONDITION:
            set_operation_phase(context, OPERATION_PHASE_COMPLETE);
            break;
        case OPERATION_PHASE_COMPLETE:
            // Terminal state - no advancement
            break;
    }

    dprintf("Advanced to operation phase: %d\n", context->current_phase);
}

/**
 * @brief Complete the current operation.
 *
 * This is a convenience function to set the operation phase to
 * OPERATION_PHASE_COMPLETE to indicate that the operation has completed.  This
 * function is useful for setting the operation phase to the complete state
 * without advancing through the operation lifecycle.
 *
 * @param context The operation context to complete.
 */
void complete_operation(operation_context_t* context) {
    set_operation_phase(context, OPERATION_PHASE_COMPLETE);
}

/**
 * @brief Set the phase state to a specific state.
 *
 * This function will set the phase state to the specified state.  This function
 * is useful for setting the phase state to a specific state without advancing
 * through the phase lifecycle.
 *
 * @param context The operation context to set the phase state for.
 * @param state The phase state to set.
 */
void set_phase_state(operation_context_t* context, phase_state_t state) {
    context->phase_state = state;
    dprintf("Set phase state: %d\n", state);
}

/**
 * @brief Advance the phase state to the next state.
 *
 * This function will advance the phase state to the next state in the phase
 * lifecycle.  If the current state is PHASE_STATE_COMPLETE or
 * PHASE_STATE_CANCELLED, this function will not advance the state, since those
 * are terminal states.
 *
 * @param context The operation context to advance the phase state for.
 */
void advance_phase_state(operation_context_t* context) {
    switch(context->phase_state) {
        case PHASE_STATE_NONE:
            set_phase_state(context, PHASE_STATE_INIT);
            break;
        case PHASE_STATE_INIT:
            set_phase_state(context, PHASE_STATE_AWAITING_INPUT);
            break;
        case PHASE_STATE_AWAITING_INPUT:
            set_phase_state(context, PHASE_STATE_PROCESSING);
            break;
        case PHASE_STATE_PROCESSING:
            set_phase_state(context, PHASE_STATE_COMPLETE);
            break;
        case PHASE_STATE_COMPLETE:
            // Completing the phase state will advance to the next operation
            // phase, if the operation is not yet complete
            complete_phase_state(context);
        case PHASE_STATE_CANCELLED:
            // Terminal state - no advancement
            break;
    }
    dprintf("Advanced to phase state: %d\n", context->phase_state);
}

/**
 * @brief Complete the current phase.
 *
 * This is a convenience function to set the phase state to PHASE_STATE_COMPLETE
 * to indicate that the phase has completed.  This function is useful for setting
 * the phase state to the complete state without advancing through the phase
 * lifecycle.
 *
 * @param context The operation context to complete the phase for.
 */
void complete_phase_state(operation_context_t* context) {
    set_phase_state(context, PHASE_STATE_COMPLETE);
    advance_operation_phase(context);
    dprintln("Completed phase state.");
}
