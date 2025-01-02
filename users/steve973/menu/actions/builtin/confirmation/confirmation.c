#include "debug.h"
#include "confirmation.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t confirmation_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Confirmation init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct confirm_config* config = operation_state.item->operation.confirm;
    if (!config) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Confirmation init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_CONFIRMATION);
    push_screen((managed_screen_t){
        .owner = MENU_OWNER,
        .is_custom = false,
        .display.content = screen,
        .refresh_interval_ms = 0
    });

    dprintln("Confirmation init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t confirmation_input(operation_context_t operation_state) {
    if (operation_state.result == OPERATION_RESULT_CANCELLED || operation_state.result == OPERATION_RESULT_ERROR) {
        dprintln("Confirmation input failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else if (operation_state.choice_made > -1) {
        pop_screen(MENU_OWNER);
    }
    dprintln("Confirmation input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t confirmation_processing(operation_context_t operation_state) {
    if (operation_state.choice_made == 0) {
        operation_state.result = OPERATION_RESULT_SUCCESS;
        dprintln("Confirmation processing passed -- advancing");
        return PHASE_RESULT_ADVANCE;
    } else {
        operation_state.result = OPERATION_RESULT_CANCELLED;
        dprintln("Confirmation processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    }
}

phase_result_t confirmation_complete(operation_context_t operation_state) {
    // Clean up, if necessary
    dprintln("Confirmation complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
