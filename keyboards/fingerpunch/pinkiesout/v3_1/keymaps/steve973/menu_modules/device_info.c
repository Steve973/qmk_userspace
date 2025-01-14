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
#include "info_config.h"
#include "display_manager/display_manager.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/**
 * Returns device manufacturer information.
 */
static const char* get_manufacturer_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", MANUFACTURER);
    return buffer;
}

/**
 * Returns keyboard product information.
 */
static const char* get_product_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", PRODUCT);
    return buffer;
}

/**
 * Returns MCU information.
 */
static const char* get_mcu_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", TOSTRING(QMK_MCU));
    return buffer;
}

/**
 * Returns VID/PID information.
 */
static const char* get_vid_pid_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04X/%04X", VENDOR_ID, PRODUCT_ID);
    return buffer;
}

/**
 * Returns device version information.
 */
static const char* get_device_version(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04X", DEVICE_VER);
    return buffer;
}

/**
 * Returns matrix size information.
 */
static const char* get_matrix_size(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%dx%d", MATRIX_ROWS, MATRIX_COLS);
    return buffer;
}

/**
 * Returns diode direction information.
 */
static const char* get_diode_direction(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", TOSTRING(DIODE_DIRECTION));
    return buffer;
}

/**
 * Represents the device information screen elements.
 */
static screen_element_t device_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Manufacturer",
            .value.get_value = get_manufacturer_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Keyboard",
            .value.get_value = get_product_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "MCU",
            .value.get_value = get_mcu_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "VID/PID",
            .value.get_value = get_vid_pid_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Device Ver",
            .value.get_value = get_device_version,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Matrix",
            .value.get_value = get_matrix_size,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Diode Dir",
            .value.get_value = get_diode_direction,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the device information screen.
 */
const screen_content_t device_info_screen = {
    .title = "Device Info",
    .elements = device_info_elements,
    .element_count = sizeof(device_info_elements) / sizeof(device_info_elements[0]),
    .default_y = 2
};
