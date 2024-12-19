#include "action.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

void handle_action(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_ACTION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const menu_item_t* item = operation_state.item;
    if (!item) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    // Show what we're doing
    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_ACTION,
        .title = item->label,
        .messages = {
            "Processing...",
            NULL,
            NULL
        }
    };
    operation_display_message(&display_config);

    // Execute the actual action and return its result
    operation_result_t (*action_function)(void**) = (operation_result_t (*)(void**))item->operation.action;
    operation_result_t result = action_function(operation_state.phase_data);
    operation_state.result = result;
}
