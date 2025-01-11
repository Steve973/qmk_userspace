#include "quantum/logging/debug.h"
#include "postcondition.h"
#include "../../../display/menu_display.h"
#include "../../../core/operation/operation_types.h"

#define POSTCONDITION_OWNER "postcondition"

phase_result_t postcondition_init(operation_context_t* operation_state) {
    operation_state->current_phase = OPERATION_PHASE_POSTCONDITION;
    // This comes after the Result phase, so the previous result should be SUCCESS
    // or NONE, if there was no Result phase defined.
    if (operation_state->result != OPERATION_RESULT_SUCCESS &&
        operation_state->result != OPERATION_RESULT_NONE) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Postcondition init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct postcondition_config* config = operation_state->item->operation.postcondition;
    if (!config || !config->handler) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Postcondition init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    // Set up the display configuration for precondition phase
    create_operation_screen(operation_state->item, OPERATION_PHASE_POSTCONDITION, POSTCONDITION_OWNER);

    dprintln("Postcondition init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t postcondition_input(operation_context_t* operation_state) {
    dprintln("Postcondition input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t postcondition_processing(operation_context_t* operation_state) {
    // Call the handler with its args
    const struct postcondition_config* config = operation_state->item->operation.postcondition;
    operation_result_t (*handler_func)(void*) = (operation_result_t (*)(void*))config->handler;
    operation_state->result = handler_func(config->args);
    remove_menu_screen(POSTCONDITION_OWNER);
    if (operation_state->result != OPERATION_RESULT_SUCCESS) {
        dprintln("Postcondition processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    }
    dprintln("Postcondition processing passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t postcondition_complete(operation_context_t* operation_state) {
    // Clean up, if necessary
    dprintln("Postcondition complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
