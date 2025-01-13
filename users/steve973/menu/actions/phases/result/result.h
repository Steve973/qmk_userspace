#pragma once

#include "../../../core/operation/operation_types.h"

phase_result_t result_init(operation_context_t* operation_state);

phase_result_t result_input(operation_context_t* operation_state);

phase_result_t result_processing(operation_context_t* operation_state);

phase_result_t result_complete(operation_context_t* operation_state);
