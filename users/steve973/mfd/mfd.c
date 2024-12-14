#include <stdbool.h>
#include <stdio.h>
#include "oled_driver.h"
#include "oled/timeout_indicator/timeout_indicator.h"
#include "timer.h"
#include "mfd.h"

static uint8_t menu_timeout_token = INVALID_DEFERRED_TOKEN;
static int32_t SCREEN_RENDERED_TIME = 0;

static const mfd_screen_t logo_screen = {
    .type = MFD_TYPE_CUSTOM,
    .refresh_interval = 0,
    .display.custom = {
        .render = oled_display_logo
    }
};

static const mfd_screen_t* current_screen = &logo_screen;
static bool current_screen_default = true;
static int8_t current_screen_index = INT8_MIN;

static void return_to_default(void) {
    current_screen_default = true;
    if (mfd_config.screen_count > 0 && mfd_config.default_index < mfd_config.screen_count) {
        mfd_config.current_index = mfd_config.default_index;
        current_screen = &mfd_config.screens[mfd_config.default_index];
        render_current_screen();
    } else {
        current_screen = &logo_screen;
        render_current_screen();
    }
}

static void render_screen_title(const char* title, bool is_default) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s%s%s",
             is_default ? "* " : "",
             title,
             is_default ? " *" : "");
    oled_write(buffer, false);
}

static void render_pairs(const mfd_screen_t* screen, uint8_t start_row, uint8_t start_col) {
    for (uint8_t i = 0; i < screen->display.simple.pair_count; i++) {
        oled_set_cursor(start_col, start_row + (i * 2));
        oled_write(screen->display.simple.pairs[i].label, false);
        oled_set_cursor(start_col, start_row + (i * 2) + 1);
        oled_write(screen->display.simple.pairs[i].get_value(), false);
    }
}

static void render_centered_pairs(const mfd_screen_t* screen) {
    uint8_t pair_count = screen->display.simple.pair_count;
    uint8_t start_row = MAX(2, (OLED_DISPLAY_HEIGHT - (pair_count * 2)) / 2);
    render_pairs(screen, start_row, 0);
}

static void render_top_aligned_pairs(const mfd_screen_t* screen) {
    render_pairs(screen, 2, 0);
}

static void render_manual_positioned_pairs(const mfd_screen_t* screen) {
    render_pairs(screen, screen->display.simple.start_row,
                        screen->display.simple.start_col);
}

void render_current_screen() {
    bool initial_render = current_screen_index != mfd_config.current_index;
    uint16_t refresh_interval = current_screen->refresh_interval;
    if (!initial_render && (refresh_interval == 0 || timer_read32() < (SCREEN_RENDERED_TIME + current_screen->refresh_interval))) {
        return;
    }

    SCREEN_RENDERED_TIME = timer_read32();

    if (current_screen->type == MFD_TYPE_CUSTOM) {
        current_screen->display.custom.render();
        return;
    }

    // Only do our rendering for simple screens
    render_screen_title(current_screen->title, current_screen_default);

    if (current_screen->position_mode == MFD_POSITION_AUTO) {
        if (current_screen->auto_align == MFD_AUTO_ALIGN_CENTER) {
            render_centered_pairs(current_screen);
        } else {  // MFD_AUTO_ALIGN_TOP
            render_top_aligned_pairs(current_screen);
        }
    } else {  // MFD_POSITION_MANUAL
        render_manual_positioned_pairs(current_screen);
    }

    // Handle timeout indicator
    if (!current_screen_default && !mfd_config.cycle_screens) {
        menu_timeout_token = timeout_indicator_create(mfd_config.timeout_ms, return_to_default);
    }
}

void mfd_switch_screen(int8_t new_index) {
    if (new_index >= mfd_config.screen_count){
        return;
    } else if (new_index == LOGO_SCREEN_INDEX) {
        current_screen = &logo_screen;
        current_screen_default = mfd_config.default_index >= mfd_config.screen_count;
    } else {
        mfd_config.current_index = new_index;
        current_screen_default = (new_index == mfd_config.default_index);
        current_screen = &mfd_config.screens[new_index];
    }
    SCREEN_RENDERED_TIME = 0;
    oled_clear();
    render_current_screen();
}

static uint32_t cycle_to_next_screen(uint32_t trigger_time, void* cb_arg) {
    if (mfd_config.cycle_screens) {
        if (mfd_config.current_index + 1 >= mfd_config.screen_count) {
            current_screen = &logo_screen;
            mfd_switch_screen(LOGO_SCREEN_INDEX);
        } else {
            mfd_switch_screen((mfd_config.current_index + 1) % mfd_config.screen_count);
        }
    }
    return mfd_config.timeout_ms;
}

void mfd_init(void) {
    if (mfd_config.screen_count > 0 && mfd_config.cycle_screens) {
        defer_exec(mfd_config.timeout_ms, cycle_to_next_screen, NULL);
    }
    if (mfd_config.screen_count == 0 || mfd_config.default_index >= mfd_config.screen_count) {
        // No default set, show logo
        mfd_switch_screen(LOGO_SCREEN_INDEX);
    } else {
        // Someone set a default, use that
        mfd_switch_screen(mfd_config.default_index);
    }
}
