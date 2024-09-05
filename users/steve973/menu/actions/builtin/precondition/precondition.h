#pragma once

#include "../../../core/operation/operation_types.h"

phase_result_t precondition_init(operation_context_t* operation_state);

phase_result_t precondition_input(operation_context_t* operation_state);

phase_result_t precondition_processing(operation_context_t* operation_state);

phase_result_t precondition_complete(operation_context_t* operation_state);
