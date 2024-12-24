#pragma once

#include "../../../common/menu_operation.h"

void confirmation_init(operation_context_t operation_state);

void confirmation_input(operation_context_t operation_state);

void confirmation_processing(operation_context_t operation_state);

void confirmation_complete(operation_context_t operation_state);
