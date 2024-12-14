#include "action.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_action(operation_result_t prev_result, void** input_values) {
    if (prev_result != OPERATION_RESULT_SUCCESS &&
        prev_result != OPERATION_RESULT_NONE) {
        return prev_result;
    }

    const struct action_config* config = (const struct action_config*)input_values[0];
    if (!config) {
        return OPERATION_RESULT_ERROR;
    }

    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_ACTION,
        .title = "Processing",
        .messages = {
            "Operation in progress...",  // Could be configurable if needed
            NULL,
            NULL
        }
    };

    operation_display_message(&display_config);
    return ((operation_result_t (*)(const struct action_config*))config)(config);
}
