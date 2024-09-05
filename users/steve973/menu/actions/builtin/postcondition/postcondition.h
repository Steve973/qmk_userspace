#pragma once

#include "../../../core/operation/operation_types.h"

phase_result_t postcondition_init(operation_context_t* operation_state);

phase_result_t postcondition_input(operation_context_t* operation_state);

phase_result_t postcondition_processing(operation_context_t* operation_state);

phase_result_t postcondition_complete(operation_context_t* operation_state);
