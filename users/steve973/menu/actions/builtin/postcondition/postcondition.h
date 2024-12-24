#pragma once

#include "../../../common/menu_operation.h"

void postcondition_init(operation_context_t operation_state);

void postcondition_input(operation_context_t operation_state);

void postcondition_processing(operation_context_t operation_state);

void postcondition_complete(operation_context_t operation_state);
