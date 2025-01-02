#include "state_manager.h"
#include "debug.h"

void state_manager_init(operation_context_t* context) {
    context->current_phase = OPERATION_PHASE_NONE;
    context->phase_state = PHASE_STATE_INIT;
}

void advance_operation_phase(operation_context_t* context) {
    switch(context->current_phase) {
        case OPERATION_PHASE_NONE:
            if (context->item->operation.precondition) {
                context->current_phase = OPERATION_PHASE_PRECONDITION;
            } else if (context->item->operation.inputs) {
                context->current_phase = OPERATION_PHASE_INPUT;
            } else if (context->item->operation.confirm) {
                context->current_phase = OPERATION_PHASE_CONFIRMATION;
            } else {
                context->current_phase = OPERATION_PHASE_ACTION;
            }
            break;
        case OPERATION_PHASE_PRECONDITION:
            if (context->item->operation.inputs) {
                context->current_phase = OPERATION_PHASE_INPUT;
            } else if (context->item->operation.confirm) {
                context->current_phase = OPERATION_PHASE_CONFIRMATION;
            } else {
                context->current_phase = OPERATION_PHASE_ACTION;
            }
            break;
        case OPERATION_PHASE_INPUT:
            if (context->item->operation.confirm) {
                context->current_phase = OPERATION_PHASE_CONFIRMATION;
            } else {
                context->current_phase = OPERATION_PHASE_ACTION;
            }
            break;
        case OPERATION_PHASE_CONFIRMATION:
            context->current_phase = OPERATION_PHASE_ACTION;
            break;
        case OPERATION_PHASE_ACTION:
            if (context->item->operation.result) {
                context->current_phase = OPERATION_PHASE_RESULT;
            } else if (context->item->operation.postcondition) {
                context->current_phase = OPERATION_PHASE_POSTCONDITION;
            } else {
                context->current_phase = OPERATION_PHASE_COMPLETE;
            }
            break;
        case OPERATION_PHASE_RESULT:
            if (context->item->operation.postcondition) {
                context->current_phase = OPERATION_PHASE_POSTCONDITION;
            } else {
                context->current_phase = OPERATION_PHASE_COMPLETE;
            }
            break;
        case OPERATION_PHASE_POSTCONDITION:
            context->current_phase = OPERATION_PHASE_COMPLETE;
            break;
        case OPERATION_PHASE_COMPLETE:
            // Terminal state - no advancement
            break;
    }

    // Reset phase state when entering new phase
    if (context->current_phase != OPERATION_PHASE_COMPLETE) {
        context->phase_state = PHASE_STATE_INIT;
    }

    dprintf("Advanced to operation phase: %d\n", context->current_phase);
}

void set_operation_phase(operation_context_t* context, operation_phase_t phase) {
    context->current_phase = phase;
    // Reset phase state when setting new phase
    context->phase_state = PHASE_STATE_INIT;
    dprintf("Set operation phase: %d\n", phase);
}

void complete_operation(operation_context_t* context) {
    set_operation_phase(context, OPERATION_PHASE_COMPLETE);
}

void advance_phase_state(operation_context_t* context) {
    switch(context->phase_state) {
        case PHASE_STATE_INIT:
            context->phase_state = PHASE_STATE_AWAITING_INPUT;
            break;
        case PHASE_STATE_AWAITING_INPUT:
            context->phase_state = PHASE_STATE_PROCESSING;
            break;
        case PHASE_STATE_PROCESSING:
            context->phase_state = PHASE_STATE_COMPLETE;
            break;
        case PHASE_STATE_COMPLETE:
        case PHASE_STATE_CANCELLED:
            // Terminal states - no advancement
            break;
    }
    dprintf("Advanced to phase state: %d\n", context->phase_state);
}

void set_phase_state(operation_context_t* context, phase_state_t state) {
    context->phase_state = state;
    dprintf("Set phase state: %d\n", state);
}

void complete_phase_state(operation_context_t* context) {
    set_phase_state(context, PHASE_STATE_COMPLETE);
}
