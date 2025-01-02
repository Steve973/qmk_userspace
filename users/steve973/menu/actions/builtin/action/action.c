#include <stdlib.h>
#include "debug.h"
#include "action.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"
#include "display_manager/display_manager.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t action_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_ACTION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Action init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const menu_item_t* item = operation_state.item;
    if (!item) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Action init failed from no item! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    dprintln("Action init passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t action_input(operation_context_t operation_state) {
    if (operation_state.result == OPERATION_RESULT_CANCELLED || operation_state.result == OPERATION_RESULT_ERROR) {
        dprintln("Action input failed from previous phase! -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else {
        dprintln("Action input passed -- advancing");
        return PHASE_RESULT_ADVANCE;
    }
}

phase_result_t action_processing(operation_context_t operation_state) {
    // Show what we're doing
    screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_ACTION);
    if (screen) {
        dprintf("Pushing screen for action: %s -- %s with elements: %d\r\n", operation_state.item->label, screen->title, screen->element_count);
        push_screen((managed_screen_t){
            .owner = MENU_OWNER,
            .is_custom = false,
            .display.content = screen,
            .refresh_interval_ms = 0
        });
    }

    // Execute the actual action and return its result
    dprintf("Executing action: %s\r\n", operation_state.item->label);
    dprintf("Action function: %s\r\n", operation_state.item->operation.action);
    operation_result_t (*action_function)(operation_result_t, void**) = (operation_result_t (*)(operation_result_t, void**))operation_state.item->operation.action;
    operation_result_t result = action_function(operation_state.result, operation_state.phase_data);
    operation_state.result = result;

    if (screen) {
        pop_screen(MENU_OWNER);
    }

    if (result == OPERATION_RESULT_CANCELLED || result == OPERATION_RESULT_ERROR) {
        dprintln("Action processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    dprintln("Action processing passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t action_complete(operation_context_t operation_state) {
    void** phase_data = operation_state.phase_data;
    uint8_t count = operation_state.item->operation.input_count;
    if (phase_data) {
        for (uint8_t i = 0; i < count; i++) {
            if (phase_data[i]) {
                free(phase_data[i]);
            }
        }
        free(phase_data);
    }
    dprintln("Action complete -- done");
    return PHASE_RESULT_COMPLETE;
}
