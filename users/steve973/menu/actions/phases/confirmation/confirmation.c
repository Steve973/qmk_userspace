#include "quantum/logging/debug.h"
#include "confirmation.h"
#include "../../../display/menu_display.h"
#include "../../../core/operation/operation_types.h"

#define CONFIRMATION_OWNER "confirmation"

phase_result_t confirmation_init(operation_context_t* operation_state) {
    operation_state->current_phase = OPERATION_PHASE_CONFIRMATION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state->result != OPERATION_RESULT_SUCCESS &&
        operation_state->result != OPERATION_RESULT_NONE) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Confirmation init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct confirm_config* config = operation_state->item->operation.confirm;
    if (!config) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Confirmation init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    create_operation_screen(operation_state->item, OPERATION_PHASE_CONFIRMATION, CONFIRMATION_OWNER);

    dprintln("Confirmation init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t confirmation_input(operation_context_t* operation_state) {
    // TODO: need to have a way to determine if any choice has been made, or if
    //       the confirmation has timed out.  In those cases, we need to return
    //       CONTINUE.  If a choice has been made, then it can be checked and
    //       return ADVANCE or CANCEL. Otherwise, it is not waiting for the user
    //       to make a choice and advances right away.
    if (operation_state->result == OPERATION_RESULT_CANCELLED || operation_state->result == OPERATION_RESULT_ERROR) {
        dprintln("Confirmation input failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else if (operation_state->choice_made > -1) {
        remove_menu_screen(CONFIRMATION_OWNER);
    }
    dprintln("Confirmation input passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t confirmation_processing(operation_context_t* operation_state) {
    dprintf("Confirmation processing: %d\n", operation_state->choice_made);
    if (operation_state->choice_made == 0) {
        operation_state->result = OPERATION_RESULT_SUCCESS;
        dprintln("Confirmation processing passed -- advancing");
        return PHASE_RESULT_ADVANCE;
    } else {
        operation_state->result = OPERATION_RESULT_CANCELLED;
        dprintln("Confirmation processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    }
}

phase_result_t confirmation_complete(operation_context_t* operation_state) {
    // Clean up, if necessary
    dprintln("Confirmation complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
