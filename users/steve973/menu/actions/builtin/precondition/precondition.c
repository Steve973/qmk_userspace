#include "debug.h"
#include "precondition.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t precondition_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_PRECONDITION;
    // Precondition is first in the chain, so prev_result should be NONE
    if (operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Precondition init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct precondition_config* config = operation_state.item->operation.precondition;
    if (!config || !config->handler) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Precondition init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    // Set up the display configuration for precondition phase
    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_PRECONDITION);
    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    dprintln("Precondition init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t precondition_input(operation_context_t operation_state) {
    dprintln("Precondition input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t precondition_processing(operation_context_t operation_state) {
    // Call the handler with its args
    const struct precondition_config* config = operation_state.item->operation.precondition;
    operation_result_t (*handler_func)(void*) = (operation_result_t (*)(void*))config->handler;
    operation_state.result = handler_func(config->args);
    pop_screen(MENU_OWNER);
    if (operation_state.result != OPERATION_RESULT_SUCCESS) {
        dprintln("Precondition processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else {
        dprintln("Precondition processing passed -- advancing");
        return PHASE_RESULT_ADVANCE;
    }
}

phase_result_t precondition_complete(operation_context_t operation_state) {
    // Clean up, if necessary
    dprintln("Precondition complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
