#include "postcondition.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_postcondition(operation_result_t prev_result, void** input_values) {
    if (prev_result != OPERATION_RESULT_SUCCESS &&
        prev_result != OPERATION_RESULT_NONE) {
        return prev_result;
    }

    const struct postcondition_config* config = (const struct postcondition_config*)input_values[0];
    if (!config) {
        return OPERATION_RESULT_ERROR;
    }

    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_POSTCONDITION,
        .title = "Verifying",
        .messages = {
            config->message,
            NULL,
            NULL
        }
    };

    operation_display_message(&display_config);
    return ((operation_result_t (*)(const struct postcondition_config*))config)(config);
}
