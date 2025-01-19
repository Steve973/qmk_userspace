#include <stdlib.h>
#include "menu/core/state/menu_state.h"
#include "quantum/logging/debug.h"
#include "menu_operation.h"
#include "../../actions/phases/precondition/precondition.h"
#include "../../actions/phases/input/input.h"
#include "../../actions/phases/confirmation/confirmation.h"
#include "../../actions/phases/action/action.h"
#include "../../actions/phases/result/result.h"
#include "../../actions/phases/postcondition/postcondition.h"
#include "../../actions/lifecycle/operation_lifecycle_manager.h"

// Internal state for the current operation
static operation_context_t operation_state;

#define HANDLE_PHASE_STATES(phase) \
    ({ \
        dprintf("Handling phase: %d, state: %d\n", operation_state.current_phase, operation_state.phase_state); \
        phase_result_t result; \
        switch (operation_state.phase_state) { \
            case PHASE_STATE_INIT:           result = phase##_init(&operation_state); break; \
            case PHASE_STATE_AWAITING_INPUT: result = phase##_input(&operation_state); break; \
            case PHASE_STATE_PROCESSING:     result = phase##_processing(&operation_state); break; \
            case PHASE_STATE_COMPLETE:       result = phase##_complete(&operation_state); break; \
            case PHASE_STATE_CANCELLED:      result = PHASE_RESULT_CANCEL; break; \
            default:                         result = PHASE_RESULT_ERROR; break; \
        } \
        result; \
    })

/**
 * @brief Handle operation lifecycle state transition based on the phase result.
 */
static void handle_phase_result(operation_context_t* context, phase_result_t result) {
    dprintf("Phase: %d, result: %d\n", context->current_phase, result);
    switch (result) {
        case PHASE_RESULT_CONTINUE:
            // Stay in current state
            break;
        case PHASE_RESULT_ADVANCE:
            advance_phase_state(context);
            break;
        case PHASE_RESULT_COMPLETE:
            advance_phase_state(context);
            break;
        case PHASE_RESULT_CANCEL:
            cancel_operation(false);
            break;
        case PHASE_RESULT_ERROR:
            cancel_operation(true);
            break;
    }
}

/**
 * @brief Start a new operation.
 *
 * This function will start a new operation with the specified menu item.  The
 * operation will be executed through its lifecycle phases until completion or
 * cancellation.
 *
 * @param item The menu item to start the operation for.
 */
bool start_operation(const menu_item_t* item) {
    if (!item || !item->operation.action) {
        dprintf("ERROR -- Cancelling operation because no action is defined for: %s\r\n", item->label);
        set_phase_state(&operation_state, PHASE_STATE_CANCELLED);
        return false;
    } else {
        operation_state = (operation_context_t){
            .current_phase = OPERATION_PHASE_NONE,
            .phase_state = PHASE_STATE_NONE,
            .item = item,
            .choice_made = -1,
            .phase_data = NULL,
            .result = OPERATION_RESULT_NONE
        };
        operation_lifecycle_manager_init(&operation_state);
        execute_operation();
        return true;
    }
}

/**
 * @brief Execute the current operation phase.
 *
 * This function will execute the current phase of the operation.  The operation
 * state must already contain the item that is in the process of being executed.
 * This happens when the start_operation function is called with the item to be
 * executed.  This function is called multiple times during the operation action
 * lifecycle to continue processing the operation through its phases and each
 * phase state.
 */
void execute_operation(void) {
    const menu_item_t* item = operation_state.item;
    dprintf("Executing operation: %s (%d)\r\n", item->label, operation_state.current_phase);
    phase_result_t phase_result = PHASE_RESULT_CONTINUE;

    if (operation_state.current_phase == OPERATION_PHASE_NONE) {
        dprintln("WARNING: Running operation phase NONE (will advance to precondition)");
        phase_result = PHASE_RESULT_ADVANCE;
    }

    if (operation_state.current_phase == OPERATION_PHASE_PRECONDITION) {
        dprintln("Running precondition phase");
        phase_result = item->operation.precondition ? HANDLE_PHASE_STATES(precondition) : PHASE_RESULT_COMPLETE;
    }

    if (operation_state.current_phase == OPERATION_PHASE_INPUT) {
        dprintln("Running input phase");
        phase_result = item->operation.inputs ? HANDLE_PHASE_STATES(input) : PHASE_RESULT_COMPLETE;
    }

    if (operation_state.current_phase == OPERATION_PHASE_CONFIRMATION) {
        dprintln("Running confirmation phase");
        phase_result = item->operation.confirm ? HANDLE_PHASE_STATES(confirmation) : PHASE_RESULT_COMPLETE;
    }

    // Action phase is required
    if (operation_state.current_phase == OPERATION_PHASE_ACTION) {
        dprintln("Running action phase");
        phase_result = HANDLE_PHASE_STATES(action);
    }

    if (operation_state.current_phase == OPERATION_PHASE_RESULT) {
        dprintln("Running result phase");
        phase_result = item->operation.result ? HANDLE_PHASE_STATES(result) : PHASE_RESULT_COMPLETE;
    }

    if (operation_state.current_phase == OPERATION_PHASE_POSTCONDITION) {
        dprintln("Running postcondition phase");
        phase_result = item->operation.postcondition ? HANDLE_PHASE_STATES(postcondition) : PHASE_RESULT_COMPLETE;
    }

    handle_phase_result(&operation_state, phase_result);
}

bool set_operation_selection(int8_t selection) {
    operation_state.choice_made = selection;
    return true;
}

operation_phase_t get_current_operation_phase(void) {
    return operation_state.current_phase;
}

bool is_operation_in_progress(void) {
    return operation_state.current_phase != OPERATION_PHASE_NONE &&
           operation_state.current_phase != OPERATION_PHASE_COMPLETE;
}

bool cancel_operation(bool is_error) {
    dprintf("Cancelling operation%s\n", is_error ? " due to error" : "");
    operation_state.result = is_error ? OPERATION_RESULT_ERROR : OPERATION_RESULT_CANCELLED;
    set_phase_state(&operation_state, PHASE_STATE_CANCELLED);
    set_operation_phase(&operation_state, OPERATION_PHASE_COMPLETE);
    return true;
}
