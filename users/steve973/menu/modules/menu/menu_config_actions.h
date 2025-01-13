#pragma once

#include "menu/core/operation/operation_types.h"

operation_result_t set_menu_selection_style(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_show_shortcuts(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_shortcut_mode(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_result_timeout(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_wasd_nav(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_confirm_timeout(operation_result_t prev_result, void** input_values);
operation_result_t set_menu_timeout_sec(operation_result_t prev_result, void** input_values);
