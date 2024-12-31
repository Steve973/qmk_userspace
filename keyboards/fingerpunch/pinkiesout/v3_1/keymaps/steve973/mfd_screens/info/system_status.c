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
#include "quantum/rgb_matrix/rgb_matrix.h"
#include "mfd/mfd.h"

static const char* get_rgb_hue_status(void) {
    static char buffer[32];
    if (rgb_matrix_is_enabled()) {
        snprintf(buffer, sizeof(buffer), "%d   ",
            rgb_matrix_get_hue());
    } else {
        snprintf(buffer, sizeof(buffer), "Off");
    }
    return buffer;
}

static const char* get_rgb_sat_status(void) {
    static char buffer[32];
    if (rgb_matrix_is_enabled()) {
        snprintf(buffer, sizeof(buffer), "%d   ",
            rgb_matrix_get_sat());
    } else {
        snprintf(buffer, sizeof(buffer), "Off");
    }
    return buffer;
}

static const char* get_rgb_val_status(void) {
    static char buffer[32];
    if (rgb_matrix_is_enabled()) {
        snprintf(buffer, sizeof(buffer), "%d   ",
            rgb_matrix_get_val());
    } else {
        snprintf(buffer, sizeof(buffer), "Off");
    }
    return buffer;
}

static const char* get_rgb_mode_status(void) {
    static char buffer[32];
    if (rgb_matrix_is_enabled()) {
        snprintf(buffer, sizeof(buffer), "%d   ",
            rgb_matrix_get_mode());
    } else {
        snprintf(buffer, sizeof(buffer), "Off");
    }
    return buffer;
}

static const char* get_uptime(void) {
    static char buffer[32];
    uint32_t elapsed = timer_read32() / 1000;
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu", elapsed/60, elapsed%60);
    return buffer;
}

static screen_element_t system_status_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 2,
        .content.key_value = {
            .label = "RGB Hue",
            .value.get_value = get_rgb_hue_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 3,
        .content.key_value = {
            .label = "RGB Sat",
            .value.get_value = get_rgb_sat_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 4,
        .content.key_value = {
            .label = "RGB Val",
            .value.get_value = get_rgb_val_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 5,
        .content.key_value = {
            .label = "RGB Mode",
            .value.get_value = get_rgb_mode_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 6,
        .content.key_value = {
            .label = "Uptime",
            .value.get_value = get_uptime,
            .is_dynamic = true
        }
    }
};

const screen_content_t system_status_screen = {
    .title = "System Status",
    .elements = system_status_elements,
    .element_count = sizeof(system_status_elements) / sizeof(system_status_elements[0]),
    .default_y = 2
};
