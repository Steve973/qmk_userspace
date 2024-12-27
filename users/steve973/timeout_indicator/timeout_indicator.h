#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "deferred_exec.h"

#define MAX_TIMEOUT_STACK 8
#define TIMEOUT_INDICATOR_REFRESH_MS 100

typedef struct {
    uint8_t id;                   // Public identifier
    uint32_t start_time;          // Time of creation
    uint32_t timeout_ms;          // Timeout duration
    deferred_token token;         // Internal QMK token
    void (*callback)(void);       // Callback defined within struct
} timeout_entry_t;

typedef struct {
    timeout_entry_t stack[MAX_TIMEOUT_STACK];
    uint8_t stack_depth;
    bool active;
} timeout_indicator_state_t;

// Public API
uint8_t timeout_indicator_create(uint32_t timeout_ms, void (*callback)(void));
void timeout_indicator_cancel(uint8_t timeout_id);
void timeout_indicator_reset(uint8_t timeout_id);
