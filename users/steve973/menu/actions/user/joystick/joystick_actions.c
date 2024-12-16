#include "joystick/fp_joystick.h"
#include "../../../common/menu_operation.h"

operation_result_t calibrate_joystick_neutral(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        calibrate_neutral_values(true);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t calibrate_joystick_range(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        calibrate_range(false);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_joystick_mode(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_index = *(uint8_t*)input_values[0];
        joystick_stick_modes mode;

        switch(selected_index) {
            case 0: mode = JOYSTICK_SM_ANALOG; break;
            case 1: mode = JOYSTICK_SM_WASD; break;
            case 2: mode = JOYSTICK_SM_ARROWS; break;
            case 3: mode = JOYSTICK_SM_MOUSE; break;
            default: return OPERATION_RESULT_ERROR;
        }

        set_stick_mode(mode);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t detect_orientation(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        // Wait for movement or escape
        if (wait_for_js_movement(10000, 85)) {
            int8_t up_direction = calculate_direction(false);
            if (up_direction != JS_NEUTRAL) {
                joystick_config.up_orientation = up_direction;
                fp_kb_config_user.js_config = joystick_config;
                fp_kb_config_save();
            }
        }
    }
    return prev_result;
}

operation_result_t set_x_axis_sensitivity(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        // TODO: Implement x-axis sensitivity operation
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_y_axis_sensitivity(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        // TODO: Implement y-axis sensitivity operation
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}
