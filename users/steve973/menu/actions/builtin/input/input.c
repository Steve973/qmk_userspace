#include "input.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_input(operation_result_t prev_result, void** input_values) {
    if (prev_result != OPERATION_RESULT_SUCCESS &&
        prev_result != OPERATION_RESULT_NONE) {
        return prev_result;
    }

    const input_config_t* config = (const input_config_t*)input_values[0];
    if (!config) {
        return OPERATION_RESULT_ERROR;
    }

    switch(config->type) {
        case INPUT_TYPE_RANGE: {
            operation_display_config_t display_config = {
                .type = OPERATION_PHASE_INPUT,
                .title = "Range Input",
                .messages = {
                    config->prompt,
                    NULL,
                    NULL
                },
                .phase_data.input = {
                    .inputs = config,
                    .input_count = get_operation_input_count()
                },
                .selected_index = -1
            };
            operation_display_message(&display_config);
            return operation_display_get_choice() >= 0 ?
                   OPERATION_RESULT_SUCCESS :
                   OPERATION_RESULT_CANCELLED;
        }
        case INPUT_TYPE_OPTIONS: {
            operation_display_config_t display_config = {
                .type = OPERATION_PHASE_INPUT,
                .title = "Select Option",
                .messages = {
                    config->prompt,
                    NULL,
                    NULL
                },
                .phase_data.input = {
                    .inputs = config,
                    .input_count = get_operation_input_count()
                },
                .selected_index = -1
            };
            operation_display_message(&display_config);
            return operation_display_get_choice() >= 0 ?
                   OPERATION_RESULT_SUCCESS :
                   OPERATION_RESULT_CANCELLED;
        }
        case INPUT_TYPE_CUSTOM:
            return ((operation_result_t (*)(const input_config_t*))input_values[0])(input_values[0]);

        default:
            return OPERATION_RESULT_ERROR;
    }
}
