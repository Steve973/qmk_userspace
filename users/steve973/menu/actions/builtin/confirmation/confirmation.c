#include "confirmation.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

void handle_confirmation(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const struct confirm_config* config = operation_state.item->operation.confirm;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
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
    operation_state.result = operation_display_get_choice() < 0 ?
        OPERATION_RESULT_CANCELLED : OPERATION_RESULT_SUCCESS;
}
