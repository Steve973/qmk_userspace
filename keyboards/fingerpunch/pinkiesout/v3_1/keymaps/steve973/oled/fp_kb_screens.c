/* Copyright 2024 Sadek Baroudi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "action_layer.h"
#include "action_util.h"
#include "quantum/led.h"
#include "tmk_core/protocol/host.h"
#include "quantum/rgb_matrix/rgb_matrix.h"
#include "wpm.h"
#include "modifiers.h"
#include "../fp_pinkiesout.h"
#include "joystick/fp_joystick.h"
#include "mfd/mfd.h"

static const char* get_layer_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s",
        get_highest_layer(layer_state|default_layer_state) == _QWERTY ? "Qwerty" :
        get_highest_layer(layer_state|default_layer_state) == _LOWER  ? "Lower" :
        get_highest_layer(layer_state|default_layer_state) == _RAISE  ? "Raise" :
        get_highest_layer(layer_state|default_layer_state) == _ADJUST ? "Adjust" :
                                                                       "Undefined");
    return buffer;
}

static const char* get_joystick_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s",
        get_stick_mode() == JOYSTICK_SM_ARROWS ? "Arrows" :
        get_stick_mode() == JOYSTICK_SM_WASD   ? "WASD" :
        get_stick_mode() == JOYSTICK_SM_ANALOG ? "Analog" :
        get_stick_mode() == JOYSTICK_SM_MOUSE  ? "Mouse" :
                                                "Undefined");
    return buffer;
}

static const char* get_led_status(void) {
    static char buffer[32];
    led_t led_state = host_keyboard_led_state();
    if (!led_state.caps_lock && !led_state.num_lock && !led_state.scroll_lock) {
        snprintf(buffer, sizeof(buffer), "None");
    } else {
        buffer[0] = '\0';
        if (led_state.caps_lock)   strcat(buffer, "CAP ");
        if (led_state.num_lock)    strcat(buffer, "NUM ");
        if (led_state.scroll_lock) strcat(buffer, "SCR");
    }
    return buffer;
}

static const char* get_mod_status(void) {
    static char buffer[32];
    uint8_t mods = get_mods();
    if (mods == 0) {
        snprintf(buffer, sizeof(buffer), "None");
    } else {
        buffer[0] = '\0';
        if (mods & MOD_MASK_SHIFT) strcat(buffer, "SHF ");
        if (mods & MOD_MASK_CTRL)  strcat(buffer, "CTL ");
        if (mods & MOD_MASK_ALT)   strcat(buffer, "ALT ");
        if (mods & MOD_MASK_GUI)   strcat(buffer, "GUI");
    }
    return buffer;
}

static const char* get_wpm_status(void) {
    static char buffer[32];
    static uint8_t peak_wpm = 0;
    uint8_t current = get_current_wpm();
    if (current > peak_wpm) {
        peak_wpm = current;
    }
    snprintf(buffer, sizeof(buffer), "%3d (%3d)", current, peak_wpm);
    return buffer;
}

static const char* get_keycount_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%lu", keypress_count);
    return buffer;
}

static const char* get_rgb_status(void) {
    static char buffer[32];
    if (rgb_matrix_is_enabled()) {
        snprintf(buffer, sizeof(buffer), "HSV:%3d,%3d,%3d M:%d",
            rgb_matrix_get_hue(),
            rgb_matrix_get_sat(),
            rgb_matrix_get_val(),
            rgb_matrix_get_mode());
    } else {
        snprintf(buffer, sizeof(buffer), "Off");
    }
    return buffer;
}

static const char* get_key_rates(void) {
    static char buffer[32];
    static uint32_t last_time = 0;
    static uint32_t last_keycount = 0;
    static uint16_t keys_per_sec = 0;
    static uint16_t keys_per_min = 0;

    uint32_t now = timer_read32();
    uint32_t elapsed = now - last_time;

    if (elapsed >= 1000) {
        keys_per_sec = keypress_count - last_keycount;
        keys_per_min = keys_per_sec * 60;
        last_keycount = keypress_count;
        last_time = now;
    }
    snprintf(buffer, sizeof(buffer), "KPS:%2d KPM:%4d", keys_per_sec, keys_per_min);
    return buffer;
}

static const char* get_uptime(void) {
    static char buffer[32];
    uint32_t elapsed = timer_read32() / 1000;
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu", elapsed/60, elapsed%60);
    return buffer;
}

static mfd_value_pair_t kb_status_pairs[] = {
    { .label = "Layer",    .get_value = get_layer_status },
    { .label = "Joystick", .get_value = get_joystick_status },
    { .label = "Lock",     .get_value = get_led_status },
    { .label = "Mods",     .get_value = get_mod_status }
};

static mfd_value_pair_t key_status_pairs[] = {
    { .label = "Keys",     .get_value = get_key_rates },
    { .label = "WPM",      .get_value = get_wpm_status },
    { .label = "Keycount", .get_value = get_keycount_status }
};

static mfd_value_pair_t system_status_pairs[] = {
    { .label = "RGB",      .get_value = get_rgb_status },
    { .label = "Uptime",   .get_value = get_uptime }
};

mfd_screen_t mfd_screens[] = {
    {
        .title = "Keyboard Status",
        .type = MFD_TYPE_SIMPLE,
        .refresh_interval = 1000,
        .position_mode = MFD_POSITION_AUTO,
        .auto_align = MFD_AUTO_ALIGN_TOP,
        .display.simple = {
            .pairs = kb_status_pairs,
            .pair_count = sizeof(kb_status_pairs) / sizeof(kb_status_pairs[0])
        }
    },
    {
        .title = "Key Stats",
        .type = MFD_TYPE_SIMPLE,
        .refresh_interval = 1000,
        .position_mode = MFD_POSITION_AUTO,
        .auto_align = MFD_AUTO_ALIGN_TOP,
        .display.simple = {
            .pairs = key_status_pairs,
            .pair_count = sizeof(key_status_pairs) / sizeof(key_status_pairs[0])
        }
    },
    {
        .title = "System Status",
        .type = MFD_TYPE_SIMPLE,
        .refresh_interval = 1000,
        .position_mode = MFD_POSITION_AUTO,
        .auto_align = MFD_AUTO_ALIGN_TOP,
        .display.simple = {
            .pairs = system_status_pairs,
            .pair_count = sizeof(system_status_pairs) / sizeof(system_status_pairs[0])
        }
    }
};

mfd_config_t mfd_config = {
    .screens = mfd_screens,
    .screen_count = sizeof(mfd_screens) / sizeof(mfd_screens[0]),
    .default_index = LOGO_SCREEN_INDEX,
    .current_index = LOGO_SCREEN_INDEX,
    .timeout_ms = 30000,
    .cycle_screens = true
};
