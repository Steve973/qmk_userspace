#pragma once

#include "../../../common/menu_operation.h"

void precondition_init(operation_context_t operation_state);

void precondition_input(operation_context_t operation_state);

void precondition_processing(operation_context_t operation_state);

void precondition_complete(operation_context_t operation_state);
