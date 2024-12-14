#include <stdbool.h>
#include <stdint.h>
#include "timer.h"
#include "oled_driver.h"
#include "timeout_indicator.h"

static uint8_t next_id = 1;

static timeout_indicator_state_t state = { .stack_depth = 0 };

static void draw_indicator(uint32_t elapsed, uint32_t timeout_ms) {
    uint8_t height = OLED_DISPLAY_HEIGHT;
    uint16_t progress = (elapsed << 8) / timeout_ms;
    uint16_t remaining_fixed = 256 - progress;
    uint8_t remaining = (height * remaining_fixed) >> 8;

    for (uint8_t i = 0; i < height; i++) {
        oled_write_pixel(OLED_DISPLAY_WIDTH-1, i, i < remaining);
    }
}

static uint32_t check_timeout(uint32_t trigger_time, void* cb_arg) {
    if (state.stack_depth == 0) return 0;

    timeout_entry_t* current = &state.stack[state.stack_depth - 1];
    uint32_t elapsed = timer_read32();

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
    new_entry->timeout_ms = timeout_ms;
    new_entry->callback = callback;
    new_entry->id = id;
    new_entry->token = defer_exec(100, check_timeout, NULL);

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
                current->token = defer_exec(100, check_timeout, NULL);
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
            state.stack[i].timeout_ms = timer_read32() + state.stack[i].timeout_ms;
            break;
        }
    }
}
