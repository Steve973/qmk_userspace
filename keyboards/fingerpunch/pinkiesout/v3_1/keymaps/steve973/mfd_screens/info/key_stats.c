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
#include "timer.h"
#include "wpm.h"
#include "fp_pinkiesout.h"
#include "mfd/mfd.h"

/**
 * Returns the current words per minute (WPM) status as a string.
 */
static const char* get_wpm_status(void) {
    static char buffer[32];
    uint8_t current = get_current_wpm();
    snprintf(buffer, sizeof(buffer), "%d   ", current);
    return buffer;
}

/**
 * Returns the peak words per minute (WPM) status as a string.
 */
static const char* get_peak_wpm_status(void) {
    static char buffer[32];
    static uint8_t peak_wpm = 0;
    uint8_t current = get_current_wpm();
    if (current > peak_wpm) {
        peak_wpm = current;
    }
    snprintf(buffer, sizeof(buffer), "%d   ", peak_wpm);
    return buffer;
}

/**
 * Returns the key count status as a string.
 */
static const char* get_keycount_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%lu   ", keypress_count);
    return buffer;
}

/**
 * Returns the keys per second (KPS) status as a string.
 */
static const char* get_kps(void) {
    static char buffer[32];
    static uint32_t last_time = 0;
    static uint32_t last_keycount = 0;
    static uint16_t keys_per_sec = 0;

    uint32_t now = timer_read32();
    uint32_t elapsed = now - last_time;

    if (elapsed >= 1000) {
        keys_per_sec = keypress_count - last_keycount;
        last_keycount = keypress_count;
        last_time = now;
    }
    snprintf(buffer, sizeof(buffer), "%d   ", keys_per_sec);
    return buffer;
}

/**
 * Returns the keys per minute (KPM) status as a string.
 */
static const char* get_kpm(void) {
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
    snprintf(buffer, sizeof(buffer), "%d   ", keys_per_min);
    return buffer;
}

/**
 * Represents the key stats screen elements.
 */
static screen_element_t key_stats_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "KPS",
            .value.get_value = get_kps,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "KPM",
            .value.get_value = get_kpm,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "WPM",
            .value.get_value = get_wpm_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Peak WPM",
            .value.get_value = get_peak_wpm_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Total",
            .value.get_value = get_keycount_status,
            .is_dynamic = true
        }
    }
};

/**
 * Represents the key stats screen.
 */
const screen_content_t key_stats_screen = {
    .title = "Key Stats",
    .elements = key_stats_elements,
    .element_count = sizeof(key_stats_elements) / sizeof(key_stats_elements[0]),
    .default_y = 2
};
