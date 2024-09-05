#pragma once

#include "menu/core/operation/operation_types.h"

operation_result_t toggle_rgb(operation_result_t prev_result, void** input_values);
operation_result_t set_rgb_mode(operation_result_t prev_result, void** input_values);
operation_result_t set_rgb_hue(operation_result_t prev_result, void** input_values);
operation_result_t set_rgb_sat(operation_result_t prev_result, void** input_values);
operation_result_t set_rgb_val(operation_result_t prev_result, void** input_values);
operation_result_t set_rgb_speed(operation_result_t prev_result, void** input_values);
