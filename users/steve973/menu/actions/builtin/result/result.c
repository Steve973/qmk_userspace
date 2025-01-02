#include "debug.h"
#include "result.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t result_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Action phase, so the previous result should be SUCCESS
    if (operation_state.result != OPERATION_RESULT_SUCCESS) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Result init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct result_config* config = operation_state.item->operation.result;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Result init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_RESULT);
    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    dprintln("Result init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_input(operation_context_t operation_state) {
    const struct result_config* config = operation_state.item->operation.result;

    if (config->mode == RESULT_MODE_ACKNOWLEDGE) {
        if (operation_state.choice_made >= 0) {
            dprintln("Result input passed -- advancing");
            return PHASE_RESULT_ADVANCE;
        }
    } else {  // RESULT_MODE_TIMED
        // Could check timeout here or let display manager handle timeout
    }
    dprintln("Result input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_processing(operation_context_t operation_state) {
    pop_screen(MENU_OWNER);
    dprintln("Result processing passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t result_complete(operation_context_t operation_state) {
    // Clean up, if necessary
    dprintln("Result complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
