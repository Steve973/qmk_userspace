#include <stdlib.h>
#include "debug.h"
#include "../actions/builtin/precondition/precondition.h"
#include "../actions/builtin/input/input.h"
#include "../actions/builtin/confirmation/confirmation.h"
#include "../actions/builtin/action/action.h"
#include "../actions/builtin/result/result.h"
#include "../actions/builtin/postcondition/postcondition.h"
#include "menu/actions/state_mgmt/state_manager.h"
#include "menu_operation.h"

// Internal state for the current operation
static operation_context_t operation_state;

#define HANDLE_PHASE_STATES(phase, result) \
    ({ \
        phase_result_t result; \
        switch (operation_state.phase_state) { \
            case PHASE_STATE_INIT:           result = phase##_init(operation_state); break; \
            case PHASE_STATE_AWAITING_INPUT: result = phase##_input(operation_state); break; \
            case PHASE_STATE_PROCESSING:     result = phase##_processing(operation_state); break; \
            case PHASE_STATE_COMPLETE:       result = phase##_complete(operation_state); break; \
            case PHASE_STATE_CANCELLED:      result = PHASE_RESULT_CANCEL; break; \
            default:                         result = PHASE_RESULT_ERROR; break; \
        } \
        result; \
    })

/**
 * @brief Handle operation lifecycle state transition based on the phase result.
 */
static void handle_phase_result(operation_context_t* context, phase_result_t result) {
    switch (result) {
        case PHASE_RESULT_CONTINUE:
            // Stay in current state
            break;
        case PHASE_RESULT_ADVANCE:
            advance_phase_state(context);
            break;
        case PHASE_RESULT_COMPLETE:
            complete_phase_state(context);
            // If phase is complete, advance to next operation phase
            if (context->phase_state == PHASE_STATE_COMPLETE) {
                advance_operation_phase(context);
            }
            break;
        case PHASE_RESULT_CANCEL:
            set_phase_state(context, PHASE_STATE_CANCELLED);
            break;
        case PHASE_RESULT_ERROR:
            context->result = OPERATION_RESULT_ERROR;
            set_phase_state(context, PHASE_STATE_CANCELLED);
            break;
    }
}

operation_result_t execute_operation(const menu_item_t* item) {
    dprintf("Executing operation: %s\r\n", item->label);
    if (!item || !item->operation.action) {
        return OPERATION_RESULT_ERROR;
    }

    operation_state = (operation_context_t){
        .current_phase = OPERATION_PHASE_NONE,
        .item = item,
        .phase_data = NULL,
        .result = OPERATION_RESULT_NONE
    };

    phase_result_t phase_result = PHASE_RESULT_ERROR;

    if (item->operation.precondition && operation_state.current_phase == OPERATION_PHASE_NONE) {
        dprintln("Running precondition phase");
        phase_result = HANDLE_PHASE_STATES(precondition, phase_result);
    } else {
        if (operation_state.current_phase == OPERATION_PHASE_NONE) {
            operation_state.current_phase = OPERATION_PHASE_INPUT;
        }
    }

    if (item->operation.inputs && operation_state.current_phase == OPERATION_PHASE_INPUT) {
        dprintln("Running input phase");
        phase_result = HANDLE_PHASE_STATES(input, phase_result);
    } else {
        if (operation_state.current_phase == OPERATION_PHASE_NONE) {
            operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
        }
    }

    if (item->operation.confirm && operation_state.current_phase == OPERATION_PHASE_CONFIRMATION) {
        dprintln("Running confirmation phase");
        phase_result = HANDLE_PHASE_STATES(confirmation, phase_result);
    } else {
        if (operation_state.current_phase == OPERATION_PHASE_NONE) {
            operation_state.current_phase = OPERATION_PHASE_ACTION;
        }
    }

    // Action phase is required
    if (operation_state.current_phase == OPERATION_PHASE_ACTION) {
        dprintln("Running action phase");
        phase_result = HANDLE_PHASE_STATES(action, phase_result);
    }

    if (item->operation.result && operation_state.current_phase == OPERATION_PHASE_RESULT) {
        dprintln("Running result phase");
        phase_result = HANDLE_PHASE_STATES(result, phase_result);
    }

    if (item->operation.postcondition && operation_state.current_phase == OPERATION_PHASE_POSTCONDITION) {
        dprintln("Running postcondition phase");
        phase_result = HANDLE_PHASE_STATES(postcondition, phase_result);
    }

    handle_phase_result(&operation_state, phase_result);

    return operation_state.result;
}

operation_phase_t get_current_operation_phase(void) {
    return operation_state.current_phase;
}

bool is_operation_in_progress(void) {
    return operation_state.current_phase != OPERATION_PHASE_NONE &&
           operation_state.current_phase != OPERATION_PHASE_COMPLETE;
}
