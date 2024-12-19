#pragma once

#include "../../../common/menu_operation.h"

operation_result_t calibrate_joystick_neutral(operation_result_t prev_result, void** input_values);
operation_result_t calibrate_joystick_range(operation_result_t prev_result, void** input_values);
operation_result_t detect_orientation(operation_result_t prev_result, void** input_values);
operation_result_t set_joystick_mode(operation_result_t prev_result, void** input_values);
operation_result_t set_x_axis_sensitivity(operation_result_t prev_result, void** input_values);
operation_result_t set_y_axis_sensitivity(operation_result_t prev_result, void** input_values);
