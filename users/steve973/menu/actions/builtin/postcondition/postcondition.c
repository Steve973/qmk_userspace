#include "postcondition.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"

void postcondition_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_POSTCONDITION;
    // This comes after the Result phase, so the previous result should be SUCCESS
    // or NONE, if there was no Result phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    const struct postcondition_config* config = operation_state.item->operation.postcondition;
    if (!config || !config->handler) {
        operation_state.result = OPERATION_RESULT_ERROR;
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    }

    // Set up the display configuration for precondition phase
    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_POSTCONDITION);
    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    operation_state.phase_state = PHASE_STATE_AWAITING_INPUT;
}

void postcondition_input(operation_context_t operation_state) {
    operation_state.phase_state = PHASE_STATE_PROCESSING;
}

void postcondition_processing(operation_context_t operation_state) {
    // Call the handler with its args
    const struct postcondition_config* config = operation_state.item->operation.postcondition;
    operation_result_t (*handler_func)(void*) = (operation_result_t (*)(void*))config->handler;
    operation_state.result = handler_func(config->args);
    pop_screen(MENU_OWNER);
    if (operation_state.result != OPERATION_RESULT_SUCCESS) {
        operation_state.phase_state = PHASE_STATE_CANCELLED;
        return;
    } else {
        operation_state.phase_state = PHASE_STATE_COMPLETE;
    }
}

void postcondition_complete(operation_context_t operation_state) {
    // Clean up, if necessary
}
