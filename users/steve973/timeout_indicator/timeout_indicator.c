#include <stdbool.h>
#include <stdint.h>
#include "timer.h"
#include "timeout_indicator.h"

static uint8_t next_id = 1;

static timeout_indicator_state_t state = { .stack_depth = 0 };

extern void draw_indicator(uint32_t elapsed, uint32_t timeout_ms);

static uint32_t check_timeout(uint32_t trigger_time, void* cb_arg) {
    if (state.stack_depth == 0) return 0;

    timeout_entry_t* current = &state.stack[state.stack_depth - 1];
    uint32_t elapsed = timer_read32() - current->start_time;

    if (elapsed >= current->timeout_ms) {
        void (*cb)(void) = current->callback;
        timeout_indicator_cancel(current->id);
        if (cb) cb();
        return 0;
    }

    draw_indicator(elapsed, current->timeout_ms);
    return TIMEOUT_INDICATOR_REFRESH_MS;
}

uint8_t timeout_indicator_create(uint32_t timeout_ms, void (*callback)(void)) {
    if (state.stack_depth >= MAX_TIMEOUT_STACK) return 0;

    // Cancel deferred exec of current top timer if it exists
    if (state.stack_depth > 0) {
        timeout_entry_t* current = &state.stack[state.stack_depth - 1];
        cancel_deferred_exec(current->token);
    }

    uint8_t id = next_id++;
    if (next_id == 0) next_id = 1;  // Wrap around, avoiding 0

    // Push new timeout to stack
    timeout_entry_t* new_entry = &state.stack[state.stack_depth++];
    *new_entry = (timeout_entry_t){
        .id = id,
        .start_time = timer_read32(),
        .timeout_ms = timeout_ms,
        .token = 0,  // Initialize to 0 or INVALID_DEFERRED_TOKEN
        .callback = callback
    };
    new_entry->token = defer_exec(TIMEOUT_INDICATOR_REFRESH_MS, check_timeout, NULL);

    return id;
}

void timeout_indicator_cancel(uint8_t timeout_id) {
    if (state.stack_depth == 0) return;

    // Find and remove the entry with this id
    for (int i = 0; i < state.stack_depth; i++) {
        if (state.stack[i].id == timeout_id) {
            cancel_deferred_exec(state.stack[i].token);

            // Shift remaining entries down
            for (int j = i; j < state.stack_depth - 1; j++) {
                state.stack[j] = state.stack[j + 1];
            }
            state.stack_depth--;

            // If there's still a timeout on the stack, start it fresh
            if (state.stack_depth > 0) {
                timeout_entry_t* current = &state.stack[state.stack_depth - 1];
                current->token = defer_exec(TIMEOUT_INDICATOR_REFRESH_MS, check_timeout, NULL);
            }
            break;
        }
    }
}

void timeout_indicator_reset(uint8_t timeout_id) {
    if (state.stack_depth == 0) return;

    // Find the entry with this id
    for (int i = 0; i < state.stack_depth; i++) {
        if (state.stack[i].id == timeout_id) {
            state.stack[i].start_time = timer_read32();
            break;
        }
    }
}
