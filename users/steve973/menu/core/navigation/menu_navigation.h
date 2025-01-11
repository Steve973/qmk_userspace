#pragma once

#include <stdbool.h>

// Core navigation functions
bool menu_home(void);
bool menu_invoke(void);
bool menu_return(void);

// Navigation queries
bool can_navigate_back(void);
