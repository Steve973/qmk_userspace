#include <stdlib.h>
#include "debug.h"
#include "input.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../../display/menu_display.h"
#include "../../../actions/state_mgmt/state_manager.h"

static int8_t input_idx = -1;

phase_result_t input_init(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_INPUT;
    if (input_idx == -1 &&
        operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Input init failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    const struct input_config* config = operation_state.item->operation.inputs;
    uint8_t input_count = operation_state.item->operation.input_count;
    if (!config || input_count == 0) {
        operation_state.result = OPERATION_RESULT_ERROR;
        dprintln("Input init failed from no config! -- cancelling");
        return PHASE_RESULT_CANCEL;
    }

    if (input_idx == -1) {
        input_idx = 0;
        operation_state.phase_data = malloc(sizeof(void*) * input_count);
        if (!operation_state.phase_data) {
            operation_state.result = OPERATION_RESULT_ERROR;
            dprintln("Input init failed from malloc! -- cancelling");
            return PHASE_RESULT_CANCEL;
        }
    }

    const struct input_config* current_input = &config[input_idx];

    switch(current_input->type) {
        case INPUT_TYPE_RANGE:
        case INPUT_TYPE_OPTIONS:
            screen_content_t* screen = create_operation_screen(operation_state.item, OPERATION_PHASE_INPUT);
            push_screen((managed_screen_t){
                .owner = MENU_OWNER,
                .is_custom = false,
                .display.content = screen,
                .refresh_interval_ms = 0
            });
            break;

        case INPUT_TYPE_CUSTOM:
            operation_result_t (*handler_func)(void*) =
                (operation_result_t (*)(void*))current_input->data.custom.handler;
            operation_result_t result = handler_func(current_input->data.custom.data);
            if (result != OPERATION_RESULT_SUCCESS) {
                operation_state.result = result;
                dprintln("Input init failed from custom handler! -- cancelling");
                return PHASE_RESULT_CANCEL;
            }
            break;

        default:
            operation_state.result = OPERATION_RESULT_ERROR;
            dprintln("Input init failed from unknown input type! -- cancelling");
            return PHASE_RESULT_CANCEL;
    }

    return PHASE_RESULT_ADVANCE;
}

phase_result_t input_input(operation_context_t operation_state) {
    if (operation_state.result == OPERATION_RESULT_CANCELLED || operation_state.result == OPERATION_RESULT_ERROR) {
        dprintln("Input input failed from previous result?! -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else if (operation_state.choice_made < 0) {
        dprintln("Input input failed from no choice! -- cancelling");
        return PHASE_RESULT_CANCEL;
    } else {
        // Store choice in already-allocated phase_data
        const struct input_config* current_input =
            &operation_state.item->operation.inputs[input_idx];

        switch(current_input->type) {
            case INPUT_TYPE_RANGE: {
                int16_t* value = malloc(sizeof(int16_t));
                *value = current_input->data.range.min +
                    (operation_state.choice_made * current_input->data.range.step);
                operation_state.phase_data[input_idx] = value;
                break;
            }
            case INPUT_TYPE_OPTIONS: {
                uint8_t* selected = malloc(sizeof(uint8_t));
                *selected = (uint8_t)operation_state.choice_made;
                operation_state.phase_data[input_idx] = selected;
                break;
            }
            case INPUT_TYPE_CUSTOM:
                // Nothing to do, since the data was added above in the previous phase
                break;
        }

        pop_screen(MENU_OWNER);

        if (input_idx + 1 < operation_state.item->operation.input_count) {
            input_idx++;
            // Need to get the next input value, so return to the init phase
            // with the incremented index for then next input
            set_phase_state(&operation_state, PHASE_STATE_INIT);
            dprintln("Input input needs to get next input value -- continuing");
            return PHASE_RESULT_CONTINUE;
        } else {
            dprintln("Input input passed -- advancing");
            return PHASE_RESULT_ADVANCE;
        }
    }
}

phase_result_t input_processing(operation_context_t operation_state) {
    // All inputs have been stored already
    // Just need to set success/error state
    operation_state.result = OPERATION_RESULT_SUCCESS;
    dprintln("Input processing passed -- advancing");
    return PHASE_RESULT_ADVANCE;
}

phase_result_t input_complete(operation_context_t operation_state) {
    input_idx = -1;
    dprintln("Input complete passed -- completing");
    return PHASE_RESULT_COMPLETE;
}
