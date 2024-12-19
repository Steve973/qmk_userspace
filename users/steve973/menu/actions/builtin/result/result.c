#include "result.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

void handle_result(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Action phase, so the previous result should be SUCCESS
    if (operation_state.result != OPERATION_RESULT_SUCCESS) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const struct result_config* config = operation_state.item->operation.result;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
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
    operation_state.result = operation_display_get_choice() < 0 ?
        OPERATION_RESULT_CANCELLED : OPERATION_RESULT_SUCCESS;
}
