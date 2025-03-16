#include "quantum/logging/debug.h"
#include "result.h"
#include "../../../display/menu_display.h"
#include "../../../core/operation/operation_types.h"

#define RESULT_OWNER "result"

phase_result_t result_init(operation_context_t* operation_state) {
    operation_state->current_phase = OPERATION_PHASE_RESULT;
    // This comes after the Action phase, so the previous result should be SUCCESS
    if (operation_state->result != OPERATION_RESULT_SUCCESS) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Result init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct result_config* config = operation_state->item->operation.result;
    if (!config) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Result init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    create_operation_screen(operation_state->item, OPERATION_PHASE_RESULT, RESULT_OWNER);

    dprintln("Result init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_input(operation_context_t* operation_state) {
    // const struct result_config* config = operation_state->item->operation.result;

    if (operation_state->choice_made >= 0) {
        dprintln("Result input passed -- advancing");
        return PHASE_RESULT_ADVANCE;
    }
    // dprintf("Result timeout: %d\n", config->base_config.timeout_sec);
    // TODO: handle timeout
    dprintln("Result input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_processing(operation_context_t* operation_state) {
    remove_menu_screen(RESULT_OWNER);
    dprintln("Result processing passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_complete(operation_context_t* operation_state) {
    // Clean up, if necessary
    dprintln("Result complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
