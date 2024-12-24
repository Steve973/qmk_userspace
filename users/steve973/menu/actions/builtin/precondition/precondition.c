#include "precondition.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"

void precondition_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_PRECONDITION;
    // Precondition is first in the chain, so prev_result should be NONE
    if (operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const struct precondition_config* config = operation_state.item->operation.precondition;
    if (!config || !config->handler) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    // Set up the display configuration for precondition phase
    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_PRECONDITION);
    push_screen((managed_screen_t){
        .owner = "menu",
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    operation_state.phase_state = PHASE_STATE_AWAITING_INPUT;
}

void precondition_input(operation_context_t operation_state) {
    operation_state.phase_state = PHASE_STATE_PROCESSING;
}

void precondition_processing(operation_context_t operation_state) {
    // Call the handler with its args
    const struct precondition_config* config = operation_state.item->operation.precondition;
    operation_result_t (*handler_func)(void*) = (operation_result_t (*)(void*))config->handler;
    operation_state.result = handler_func(config->args);
    pop_screen("menu");
    operation_state.phase_state = PHASE_STATE_COMPLETE;
}

void precondition_complete(operation_context_t operation_state) {
    // Clean up, if necessary
}
