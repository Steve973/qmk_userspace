#include "precondition.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_precondition(operation_result_t prev_result, void** input_values) {
    // Precondition is first in the chain, so prev_result should be NONE
    if (prev_result != OPERATION_RESULT_NONE) {
        return prev_result;
    }

    const struct precondition_config* config = (const struct precondition_config*)input_values[0];
    if (!config || !config->handler) {
        return OPERATION_RESULT_ERROR;
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
    operation_result_t result = handler_func(config->args);

    return result;
}
