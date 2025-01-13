#pragma once

#include "../../../core/operation/operation_types.h"

phase_result_t input_init(operation_context_t* operation_state);

phase_result_t input_input(operation_context_t* operation_state);

phase_result_t input_processing(operation_context_t* operation_state);

phase_result_t input_complete(operation_context_t* operation_state);
