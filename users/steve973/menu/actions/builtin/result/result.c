#include "result.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"

void result_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Action phase, so the previous result should be SUCCESS
    if (operation_state.result != OPERATION_RESULT_SUCCESS) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    const struct result_config* config = operation_state.item->operation.result;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_RESULT);
    push_screen((managed_screen_t){
        .owner = "menu",
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    operation_state.phase_state = PHASE_STATE_AWAITING_INPUT;
}

void result_input(operation_context_t operation_state) {
    const struct result_config* config = operation_state.item->operation.result;

    if (config->mode == RESULT_MODE_ACKNOWLEDGE) {
        if (operation_state.choice_made >= 0) {
            operation_state.phase_state = PHASE_STATE_PROCESSING;
        }
    } else {  // RESULT_MODE_TIMED
        // Could check timeout here or let display manager handle timeout
        operation_state.phase_state = PHASE_STATE_PROCESSING;
    }
}

void result_processing(operation_context_t operation_state) {
    pop_screen("menu");
    operation_state.phase_state = PHASE_STATE_COMPLETE;
}

void result_complete(operation_context_t operation_state) {
    // Clean up, if necessary
}
