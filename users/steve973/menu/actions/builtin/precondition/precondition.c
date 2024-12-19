#include "precondition.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

void handle_precondition(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_PRECONDITION;
    // Precondition is first in the chain, so prev_result should be NONE
    if (operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const struct precondition_config* config = operation_state.item->operation.precondition;
    if (!config || !config->handler) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    // Set up the display configuration for precondition phase
    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_PRECONDITION,
        .title = "Precondition",
        .messages = {
            config->message,  // Display the configured message
            NULL,
            NULL
        }
    };

    // Show the precondition UI
    operation_display_message(&display_config);

    // Call the handler with its args
    operation_result_t (*handler_func)(void*) = (operation_result_t (*)(void*))config->handler;
    operation_state.result = handler_func(config->args);
}
