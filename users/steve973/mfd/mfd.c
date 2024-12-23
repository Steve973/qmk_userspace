#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "deferred_exec.h"
#include "display_manager/display_manager.h"
#include "mfd.h"

static const managed_screen_t logo_screen = {
    .owner = "mfd",
    .display.render = &oled_display_logo,
    .refresh_interval_ms = 0,
    .is_custom = true
};

void mfd_switch_screen(int8_t new_index) {
    if (new_index >= mfd_config.screen_count) {
        return;
    }
    mfd_config.current_index = new_index;
    if (new_index == LOGO_SCREEN_INDEX) {
        swap_screen(logo_screen);
    } else {
        swap_screen((managed_screen_t) {
            .owner = "mfd",
            .is_custom = false,
            .display.content = &mfd_config.screens[new_index],
            .refresh_interval_ms = 200
        });
    }
}

void increment_screen(bool positive_increment) {
    int8_t increment = positive_increment ? 1 : -1;
    int8_t new_index = mfd_config.current_index + increment;
    if (new_index >= mfd_config.screen_count) {
        new_index = LOGO_SCREEN_INDEX;
    } else if (new_index < LOGO_SCREEN_INDEX) {
        new_index = mfd_config.screen_count - 1;
    }
    mfd_switch_screen(new_index);
}

static uint32_t cycle_to_next_screen(uint32_t trigger_time, void* cb_arg) {
    increment_screen(true);
    return mfd_config.cycle_screens ? mfd_config.timeout_ms : 0;
}

void mfd_init(void) {
    if (mfd_config.screen_count > 0 && mfd_config.cycle_screens) {
        defer_exec(10, cycle_to_next_screen, NULL);
    } else if (mfd_config.screen_count == 0 || mfd_config.default_index >= mfd_config.screen_count) {
        // No default set, show logo
        mfd_switch_screen(LOGO_SCREEN_INDEX);
    } else {
        // Someone set a default, use that
        mfd_switch_screen(mfd_config.default_index);
    }
}
