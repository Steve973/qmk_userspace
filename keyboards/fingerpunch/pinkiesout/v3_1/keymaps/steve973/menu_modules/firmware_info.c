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
#include <string.h>
#include "version.h"
#include "display_manager/display_manager.h"

/**
 * Returns firmware version information.
 */
static const char* get_firmware_version(void) {
    static char buffer[32];
    char version[28];
    strncpy(version, QMK_VERSION, sizeof(version) - 1);
    version[sizeof(version) - 1] = '\0';
    char* delim_pos = strchr(version, '-');
    if (delim_pos) *delim_pos = '\0';
    snprintf(buffer, sizeof(buffer), "%s", version);
    return buffer;
}

/**
 * Returns git hash information.
 */
static const char* get_git_hash(void) {
    static char buffer[32];
    char git_hash[14];
    strncpy(git_hash, QMK_GIT_HASH, sizeof(git_hash) - 1);
    git_hash[sizeof(git_hash) - 1] = '\0';
    char* delim_pos = strchr(git_hash, '*');
    if (delim_pos) *delim_pos = '\0';
    snprintf(buffer, sizeof(buffer), "%s", git_hash);
    return buffer;
}

/**
 * Returns build date information.
 */
static const char* get_build_date(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", QMK_BUILDDATE);
    return buffer;
}

/**
 * Represents the firmware information screen elements.
 */
static screen_element_t firmware_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Version",
            .value.get_value = get_firmware_version,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Git Hash",
            .value.get_value = get_git_hash,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Built",
            .value.get_value = get_build_date,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the firmware information screen.
 */
const screen_content_t firmware_info_screen = {
    .title = "Firmware Info",
    .elements = firmware_info_elements,
    .element_count = sizeof(firmware_info_elements) / sizeof(firmware_info_elements[0]),
    .default_y = 2
};
