#include <stdlib.h>
#include "menu_operation.h"
#include "../actions/builtin/precondition/precondition.h"
#include "../actions/builtin/input/input.h"
#include "../actions/builtin/confirmation/confirmation.h"
#include "../actions/builtin/action/action.h"
#include "../actions/builtin/result/result.h"
#include "../actions/builtin/postcondition/postcondition.h"
#include "debug.h"

// Internal state for the current operation
static operation_context_t operation_state;

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

    // Run through all phases, passing operation context/state forward
    if (item->operation.precondition) {
        dprint("Running precondition phase\r\n");
        switch (operation_state.phase_state) {
            case PHASE_STATE_INIT:
                precondition_init(operation_state);
                break;
            case PHASE_STATE_AWAITING_INPUT:
                precondition_input(operation_state);
                break;
            case PHASE_STATE_PROCESSING:
                precondition_processing(operation_state);
                break;
            case PHASE_STATE_COMPLETE:
                precondition_complete(operation_state);
                break;
            case PHASE_STATE_CANCELLED:
                break;
        }
    }

    if (item->operation.inputs) {
        dprint("Running input phase\r\n");
        switch (operation_state.phase_state) {
            case PHASE_STATE_INIT:
                input_init(operation_state);
                break;
            case PHASE_STATE_AWAITING_INPUT:
                input_input(operation_state);
                break;
            case PHASE_STATE_PROCESSING:
                input_processing(operation_state);
                break;
            case PHASE_STATE_COMPLETE:
                input_complete(operation_state);
                break;
            case PHASE_STATE_CANCELLED:
                break;
        }
    }

    if (item->operation.confirm) {
        dprint("Running confirmation phase\r\n");
        switch (operation_state.phase_state) {
            case PHASE_STATE_INIT:
                confirmation_init(operation_state);
                break;
            case PHASE_STATE_AWAITING_INPUT:
                confirmation_input(operation_state);
                break;
            case PHASE_STATE_PROCESSING:
                confirmation_processing(operation_state);
                break;
            case PHASE_STATE_COMPLETE:
                confirmation_complete(operation_state);
                break;
            case PHASE_STATE_CANCELLED:
                break;
        }
    }

    // Action phase is required
    dprint("Running action phase\r\n");
    switch (operation_state.phase_state) {
        case PHASE_STATE_INIT:
            action_init(operation_state);
            break;
        case PHASE_STATE_AWAITING_INPUT:
            action_input(operation_state);
            break;
        case PHASE_STATE_PROCESSING:
            action_processing(operation_state);
            break;
        case PHASE_STATE_COMPLETE:
            action_complete(operation_state);
            break;
        case PHASE_STATE_CANCELLED:
            break;
    }

    if (item->operation.result) {
        dprint("Running result phase\r\n");
        switch (operation_state.phase_state) {
            case PHASE_STATE_INIT:
                result_init(operation_state);
                break;
            case PHASE_STATE_AWAITING_INPUT:
                result_input(operation_state);
                break;
            case PHASE_STATE_PROCESSING:
                result_processing(operation_state);
                break;
            case PHASE_STATE_COMPLETE:
                result_complete(operation_state);
                break;
            case PHASE_STATE_CANCELLED:
                break;
        }
    }

    if (item->operation.postcondition) {
        dprint("Running postcondition phase\r\n");
        switch (operation_state.phase_state) {
            case PHASE_STATE_INIT:
                postcondition_init(operation_state);
                break;
            case PHASE_STATE_AWAITING_INPUT:
                postcondition_input(operation_state);
                break;
            case PHASE_STATE_PROCESSING:
                postcondition_processing(operation_state);
                break;
            case PHASE_STATE_COMPLETE:
                postcondition_complete(operation_state);
                break;
            case PHASE_STATE_CANCELLED:
                break;
        }
    }

    operation_state.current_phase = OPERATION_PHASE_COMPLETE;
    return operation_state.result;
}
// Helper function to get current phase.
operation_phase_t get_current_operation_phase(void) {
    return operation_state.current_phase;
}

// Helper to check if an operation is in progress.
bool is_operation_in_progress(void) {
    return operation_state.current_phase != OPERATION_PHASE_NONE &&
           operation_state.current_phase != OPERATION_PHASE_COMPLETE;
}
