#pragma once

#include "../../../core/operation/operation_types.h"

phase_result_t confirmation_init(operation_context_t* operation_state);

phase_result_t confirmation_input(operation_context_t* operation_state);

phase_result_t confirmation_processing(operation_context_t* operation_state);

phase_result_t confirmation_complete(operation_context_t* operation_state);
