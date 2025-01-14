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
#include "quantum/audio/audio.h"
#include "rgb_matrix/rgb_matrix.h"
#include "keycode_config.h"
#include "display_manager/display_manager.h"

/**
 * Returns RGB matrix status.
 */
static const char* get_rgb_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", rgb_matrix_is_enabled() ? "On" : "Off");
    return buffer;
}

/**
 * Returns audio status.
 */
static const char* get_audio_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", audio_is_on() ? "On" : "Off");
    return buffer;
}

/**
 * Returns NKRO status.
 */
static const char* get_nkro_status(void) {
    static char buffer[32];
    #ifdef NKRO_ENABLE
    snprintf(buffer, sizeof(buffer), "%s", keymap_config.nkro ? "On" : "Off");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Returns mousekey status.
 */
static const char* get_mousekey_status(void) {
    static char buffer[32];
    #ifdef MOUSEKEY_ENABLE
    snprintf(buffer, sizeof(buffer), "Enabled");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Returns extrakey status.
 */
static const char* get_extrakey_status(void) {
    static char buffer[32];
    #ifdef EXTRAKEY_ENABLE
    snprintf(buffer, sizeof(buffer), "Enabled");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Represents the feature status screen elements.
 */
static screen_element_t feature_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "RGB Matrix",
            .value.get_value = get_rgb_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Audio",
            .value.get_value = get_audio_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "NKRO",
            .value.get_value = get_nkro_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Mouse Keys",
            .value.get_value = get_mousekey_status,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Extra Keys",
            .value.get_value = get_extrakey_status,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the feature status screen.
 */
const screen_content_t feature_info_screen = {
    .title = "Features",
    .elements = feature_info_elements,
    .element_count = sizeof(feature_info_elements) / sizeof(feature_info_elements[0]),
    .default_y = 2
};
