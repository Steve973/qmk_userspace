#pragma once

#include "menu/core/operation/operation_types.h"

operation_result_t reset_eeprom(operation_result_t prev_result, void** input_values);
operation_result_t enter_bootloader(operation_result_t prev_result, void** input_values);
operation_result_t test_piezo(operation_result_t prev_result, void** input_values);
operation_result_t increase_oled_brightness(operation_result_t prev_result, void** input_values);
operation_result_t decrease_oled_brightness(operation_result_t prev_result, void** input_values);
operation_result_t set_oled_brightness(operation_result_t prev_result, void** input_values);
operation_result_t show_debug_info(operation_result_t prev_result, void** input_values);
operation_result_t show_kb_stats(operation_result_t prev_result, void** input_values);
