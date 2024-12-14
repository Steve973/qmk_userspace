#include <stdlib.h>
#include "menu_operation.h"
#include "../actions/builtin/precondition/precondition.h"
#include "../actions/builtin/input/input.h"
#include "../actions/builtin/confirmation/confirmation.h"
#include "../actions/builtin/action/action.h"
#include "../actions/builtin/result/result.h"
#include "../actions/builtin/postcondition/postcondition.h"

// Internal state for the current operation
static struct {
    operation_phase_t current_phase;
    void** input_values;
    uint8_t input_count;
} operation_state;

operation_result_t execute_operation(const menu_item_t* item) {
    if (!item || !item->operation.action) {
        return OPERATION_RESULT_ERROR;
    }

    operation_result_t result = OPERATION_RESULT_NONE;

    // Initialize input values array if needed
    operation_state.input_values = NULL;
    operation_state.input_count = 0;

    if (item->operation.inputs && item->operation.input_count > 0) {
        operation_state.input_values = calloc(item->operation.input_count, sizeof(void*));
        operation_state.input_count = item->operation.input_count;
        if (!operation_state.input_values) {
            return OPERATION_RESULT_ERROR;
        }
    }

    // Run through all phases, passing result forward
    if (item->operation.precondition) {
        operation_state.current_phase = OPERATION_PHASE_PRECONDITION;
        result = handle_precondition(result, operation_state.input_values);
    }

    if (item->operation.inputs) {
        operation_state.current_phase = OPERATION_PHASE_INPUT;
        result = handle_input(result, operation_state.input_values);
    }

    if (item->operation.confirm) {
        operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
        result = handle_confirmation(result, operation_state.input_values);
    }

    // Action phase is required
    operation_state.current_phase = OPERATION_PHASE_ACTION;
    result = handle_action(result, operation_state.input_values);

    if (item->operation.result) {
        operation_state.current_phase = OPERATION_PHASE_RESULT;
        result = handle_result(result, operation_state.input_values);
    }

    if (item->operation.postcondition) {
        operation_state.current_phase = OPERATION_PHASE_POSTCONDITION;
        result = handle_postcondition(result, operation_state.input_values);
    }

    operation_state.current_phase = OPERATION_PHASE_COMPLETE;
    return result;
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

// Helper to get the number of input values for the current operation.
uint8_t get_operation_input_count(void) {
    return operation_state.input_count;
}
