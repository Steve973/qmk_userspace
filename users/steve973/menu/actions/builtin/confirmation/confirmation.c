#include "confirmation.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"

void confirmation_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    const struct confirm_config* config = operation_state.item->operation.confirm;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_CONFIRMATION);
    push_screen((managed_screen_t){
        .owner = "menu",
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    operation_state.phase_state = PHASE_STATE_AWAITING_INPUT;
}

void confirmation_input(operation_context_t operation_state) {
    if (operation_state.result == OPERATION_RESULT_CANCELLED || operation_state.result == OPERATION_RESULT_ERROR) {
        operation_state.phase_state = PHASE_STATE_CANCELLED;
    } else if (operation_state.choice_made > -1) {
        pop_screen("menu");
        operation_state.phase_state = PHASE_STATE_PROCESSING;
    }
}

void confirmation_processing(operation_context_t operation_state) {
    if (operation_state.choice_made == 0) {
        operation_state.result = OPERATION_RESULT_SUCCESS;
        operation_state.phase_state = PHASE_STATE_COMPLETE;
    } else {
        operation_state.result = OPERATION_RESULT_CANCELLED;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
    }
}

void confirmation_complete(operation_context_t operation_state) {
    // Clean up, if necessary
}
