#include "result.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_result(operation_result_t prev_result, void** input_values) {
    const struct result_config* config = (const struct result_config*)input_values[0];
    if (!config) {
        return OPERATION_RESULT_ERROR;
    }

    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_RESULT,
        .title = "Result",
        .messages = {
            config->message,
            NULL,
            NULL
        },
        .phase_data.result = {
            .mode = config->mode,
            .ok_text = config->ok_text
        }
    };

    operation_display_message(&display_config);
    return operation_display_get_choice() >= 0 ?
           OPERATION_RESULT_SUCCESS :
           OPERATION_RESULT_CANCELLED;
}
