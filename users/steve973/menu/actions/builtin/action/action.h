#pragma once

#include "../../../common/menu_operation.h"
#include "../../../actions/state_mgmt/state_manager.h"

phase_result_t action_init(operation_context_t operation_state);

phase_result_t action_input(operation_context_t operation_state);

phase_result_t action_processing(operation_context_t operation_state);

phase_result_t action_complete(operation_context_t operation_state);
