#include "action.h"
#include <stdlib.h>
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"

void action_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_ACTION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    const menu_item_t* item = operation_state.item;
    if (!item) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    operation_state.phase_state = PHASE_STATE_AWAITING_INPUT;
}

void action_input(operation_context_t operation_state) {
    if (operation_state.result == OPERATION_RESULT_CANCELLED || operation_state.result == OPERATION_RESULT_ERROR) {
        operation_state.phase_state = PHASE_STATE_CANCELLED;
    } else {
        operation_state.phase_state = PHASE_STATE_PROCESSING;
    }
}

void action_processing(operation_context_t operation_state) {
    // Show what we're doing
    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_ACTION);
    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    // Execute the actual action and return its result
    operation_result_t (*action_function)(void**) = (operation_result_t (*)(void**))operation_state.item->operation.action;
    operation_result_t result = action_function(operation_state.phase_data);
    operation_state.result = result;
    pop_screen(MENU_OWNER);
    if (result == OPERATION_RESULT_CANCELLED || result == OPERATION_RESULT_ERROR) {
        operation_state.phase_state = PHASE_STATE_CANCELLED;
    } else {
        operation_state.phase_state = PHASE_STATE_COMPLETE;
    }
}

void action_complete(operation_context_t operation_state) {
    void** phase_data = operation_state.phase_data;
    uint8_t count = operation_state.item->operation.input_count;
    if (!phase_data) return;
    for (uint8_t i = 0; i < count; i++) {
        if (phase_data[i]) {
            free(phase_data[i]);
        }
    }
    free(phase_data);
}
