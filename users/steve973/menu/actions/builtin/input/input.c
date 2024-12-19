#include <stdlib.h>
#include "input.h"
#include "../../../common/menu_core.h"
#include "../../../common/menu_operation.h"
#include "../../display/operation_display.h"

static void cleanup_phase_data(void** phase_data, uint8_t count) {
    if (!phase_data) return;
    for (uint8_t i = 0; i < count; i++) {
        if (phase_data[i]) {
            free(phase_data[i]);
        }
    }
    free(phase_data);
}

void handle_input(operation_context_t operation_state) {
    operation_state.current_phase = OPERATION_PHASE_INPUT;
    if (operation_state.result != OPERATION_RESULT_SUCCESS &&
        operation_state.result != OPERATION_RESULT_NONE) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    const struct input_config* config = operation_state.item->operation.inputs;
    uint8_t input_count = operation_state.item->operation.input_count;
    if (!config || input_count == 0) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    operation_state.phase_data = malloc(sizeof(void*) * input_count);
    if (!operation_state.phase_data) {
        operation_state.result = OPERATION_RESULT_ERROR;
        return;
    }

    for (uint8_t i = 0; i < input_count; i++) {
        const struct input_config* current_input = &config[i];

        switch(current_input->type) {
            case INPUT_TYPE_RANGE: {
                operation_display_config_t display_config = {
                    .type = OPERATION_PHASE_INPUT,
                    .title = "Range Input",
                    .messages = {
                        current_input->prompt,
                        NULL,
                        NULL
                    },
                    .phase_data.input = {
                        .inputs = current_input,
                        .input_count = input_count
                    },
                    .selected_index = i
                };
                operation_display_message(&display_config);
                int8_t choice = operation_display_get_choice();
                if (choice < 0) {
                    cleanup_phase_data(operation_state.phase_data, i);
                    operation_state.phase_data = NULL;
                    operation_state.result = OPERATION_RESULT_CANCELLED;
                    return;
                }

                int16_t* value = malloc(sizeof(int16_t));
                *value = current_input->data.range.min +
                    (choice * current_input->data.range.step);
                operation_state.phase_data[i] = value;
                break;
            }

            case INPUT_TYPE_OPTIONS: {
                operation_display_config_t display_config = {
                    .type = OPERATION_PHASE_INPUT,
                    .title = "Select Option",
                    .messages = {
                        current_input->prompt,
                        NULL,
                        NULL
                    },
                    .phase_data.input = {
                        .inputs = current_input,
                        .input_count = input_count
                    },
                    .selected_index = i
                };
                operation_display_message(&display_config);
                int8_t choice = operation_display_get_choice();
                if (choice < 0) {
                    cleanup_phase_data(operation_state.phase_data, i);
                    operation_state.phase_data = NULL;
                    operation_state.result = OPERATION_RESULT_CANCELLED;
                    return;
                }

                uint8_t* selected = malloc(sizeof(uint8_t));
                *selected = (uint8_t)choice;
                operation_state.phase_data[i] = selected;
                break;
            }

            case INPUT_TYPE_CUSTOM: {
                operation_result_t (*handler_func)(void*) =
                    (operation_result_t (*)(void*))current_input->data.custom.handler;
                operation_result_t result = handler_func(current_input->data.custom.data);
                if (result != OPERATION_RESULT_SUCCESS) {
                    cleanup_phase_data(operation_state.phase_data, i);
                    operation_state.phase_data = NULL;
                    operation_state.result = result;
                    return;
                }
                // Let the custom handler store its result in phase_data directly
                break;
            }

            default:
                cleanup_phase_data(operation_state.phase_data, i);
                operation_state.phase_data = NULL;
                operation_state.result = OPERATION_RESULT_ERROR;
                return;
        }
    }

    operation_state.result = OPERATION_RESULT_SUCCESS;
}
