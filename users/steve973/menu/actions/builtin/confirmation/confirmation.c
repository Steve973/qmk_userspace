#include "confirmation.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

operation_result_t handle_confirmation(operation_result_t prev_result, void** input_values) {
    if (prev_result != OPERATION_RESULT_SUCCESS &&
        prev_result != OPERATION_RESULT_NONE) {
        return prev_result;
    }

    const struct confirm_config* config = (const struct confirm_config*)input_values[0];
    if (!config) {
        return OPERATION_RESULT_ERROR;
    }

    operation_display_config_t display_config = {
        .type = OPERATION_PHASE_CONFIRMATION,
        .title = "Confirm",
        .messages = {
            config->message,
            NULL,
            NULL
        },
        .phase_data.confirm = {
            .true_text = config->true_text,
            .false_text = config->false_text
        }
    };

    operation_display_message(&display_config);
    return operation_display_get_choice() >= 0 ?
           OPERATION_RESULT_SUCCESS :
           OPERATION_RESULT_CANCELLED;
}
