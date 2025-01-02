#pragma once

#include "../../../common/menu_operation.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t postcondition_init(operation_context_t operation_state);

phase_result_t postcondition_input(operation_context_t operation_state);

phase_result_t postcondition_processing(operation_context_t operation_state);

phase_result_t postcondition_complete(operation_context_t operation_state);
