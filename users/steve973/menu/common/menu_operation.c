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
        handle_precondition(operation_state);
    }

    if (item->operation.inputs) {
        dprint("Running input phase\r\n");
        handle_input(operation_state);
    }

    if (item->operation.confirm) {
        dprint("Running confirmation phase\r\n");
        handle_confirmation(operation_state);
    }

    // Action phase is required
    dprint("Running action phase\r\n");
    handle_action(operation_state);

    if (item->operation.result) {
        dprint("Running result phase\r\n");
        handle_result(operation_state);
    }

    if (item->operation.postcondition) {
        dprint("Running postcondition phase\r\n");
        handle_postcondition(operation_state);
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
