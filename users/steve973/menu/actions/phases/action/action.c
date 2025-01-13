#include <stdlib.h>
#include "quantum/logging/debug.h"
#include "action.h"
#include "../../../display/menu_display.h"
#include "../../../core/operation/operation_types.h"
#include "../../../core/structure/menu_item.h"
#include "../../../core/generated/menu_action_lookup.h"

#define ACTION_OWNER "action"

static action_func_t find_action_function(const char* action_name) {
    for (size_t i = 0; i < action_lookup_table_size; i++) {
        if (strcmp(action_lookup_entries[i].name, action_name) == 0) {
            return action_lookup_entries[i].func;
        }
    }
    return NULL;
}

phase_result_t action_init(operation_context_t* operation_state) {
    operation_state->current_phase = OPERATION_PHASE_ACTION;
    // This comes after the Input phase, so the previous result should be SUCCESS
    // or NONE, if there was no Input phase defined.
    if (operation_state->result != OPERATION_RESULT_SUCCESS &&
        operation_state->result != OPERATION_RESULT_NONE) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Action init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const menu_item_t* item = operation_state->item;
    if (!item) {
        operation_state->result = OPERATION_RESULT_ERROR;
        dprintln("Action init failed from no item! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    dprintln("Action init passed -- advancing");
    operation_state->result = OPERATION_RESULT_SUCCESS;
    return PHASE_RESULT_ADVANCE;
}

phase_result_t action_input(operation_context_t* operation_state) {
    if (operation_state->result == OPERATION_RESULT_CANCELLED || operation_state->result == OPERATION_RESULT_ERROR) {
        dprintln("Action input failed from previous phase! -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else {
        dprintln("Action input passed -- advancing");
        operation_state->result = OPERATION_RESULT_SUCCESS;
        return PHASE_RESULT_ADVANCE;
    }
}

phase_result_t action_processing(operation_context_t* operation_state) {
    dprintf("Processing action for: %s\r\n", operation_state->item->label);

    // Look up and execute the action
    dprintf("Executing action: %s\r\n", operation_state->item->label);
    dprintf("Action function: %s\r\n", operation_state->item->operation.action);
    dprintf("Previous result: %d\r\n", operation_state->result);

    action_func_t action_function = find_action_function(operation_state->item->operation.action);
    if (!action_function) {
        dprintln("Action function not found -- cancelling");
        operation_state->result = OPERATION_RESULT_ERROR;
        return PHASE_RESULT_CANCEL;
    }

    // Show what we're doing
    screen_push_status_t push_status = create_operation_screen(operation_state->item, OPERATION_PHASE_ACTION, ACTION_OWNER);
    bool screen = push_status == SCREEN_PUSH_SUCCESS;

    operation_result_t result = action_function(operation_state->result, operation_state->phase_data);
    operation_state->result = result;

    if (screen) {
        remove_menu_screen(ACTION_OWNER);
    }

    if (result == OPERATION_RESULT_CANCELLED || result == OPERATION_RESULT_ERROR) {
        dprintln("Action processing failed -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    dprintf("Action processing passed, result: %d -- advancing\r\n", result);
    return PHASE_RESULT_ADVANCE;
}

phase_result_t action_complete(operation_context_t* operation_state) {
    void** phase_data = operation_state->phase_data;
    uint8_t count = operation_state->item->operation.input_count;
    if (phase_data) {
        for (uint8_t i = 0; i < count; i++) {
            if (phase_data[i]) {
                free(phase_data[i]);
            }
        }
        free(phase_data);
    }
    dprintln("Action complete -- done");
    dprintf("Action result: %d\r\n", operation_state->result);
    return PHASE_RESULT_COMPLETE;
}
