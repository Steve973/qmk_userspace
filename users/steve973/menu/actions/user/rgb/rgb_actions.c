#include "quantum/rgb_matrix/rgb_matrix.h"
#include "../../../common/menu_operation.h"

operation_result_t toggle_rgb(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        rgb_matrix_toggle_noeeprom();
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_rgb_mode(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_mode = *(uint8_t*)input_values[0];
        rgb_matrix_mode_noeeprom(selected_mode);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_rgb_hue(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_hue = *(uint8_t*)input_values[0];
        HSV current_hsv = rgb_matrix_get_hsv();
        rgb_matrix_sethsv_noeeprom(selected_hue, current_hsv.s, current_hsv.v);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_rgb_sat(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_sat = *(uint8_t*)input_values[0];
        HSV current_hsv = rgb_matrix_get_hsv();
        rgb_matrix_sethsv_noeeprom(current_hsv.h, selected_sat, current_hsv.v);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_rgb_val(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_val = *(uint8_t*)input_values[0];
        HSV current_hsv = rgb_matrix_get_hsv();
        rgb_matrix_sethsv_noeeprom(current_hsv.h, current_hsv.s, selected_val);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t set_rgb_speed(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        uint8_t selected_speed = *(uint8_t*)input_values[0];
        rgb_matrix_set_speed_noeeprom(selected_speed);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

