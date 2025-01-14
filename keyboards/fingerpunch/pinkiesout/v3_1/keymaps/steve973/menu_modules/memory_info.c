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
#include "display_manager/display_manager.h"

/**
 * Returns bootloader size information.
 */
static const char* get_bootloader_size(void) {
    static char buffer[32];
    #ifdef BOOTLOADER_SIZE
    snprintf(buffer, sizeof(buffer), "%dKB", BOOTLOADER_SIZE / 1024);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Returns firmware size information.
 */
static const char* get_firmware_size(void) {
    static char buffer[32];
    #ifdef FIRMWARE_SIZE
    snprintf(buffer, sizeof(buffer), "%dKB", FIRMWARE_SIZE / 1024);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Returns EEPROM size information.
 */
static const char* get_eeprom_size(void) {
    static char buffer[32];
    #ifdef EEPROM_SIZE
    snprintf(buffer, sizeof(buffer), "%dB", EEPROM_SIZE);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Represents the memory information screen elements.
 */
static screen_element_t memory_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Boot Size",
            .value.get_value = get_bootloader_size,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "FW Size",
            .value.get_value = get_firmware_size,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "EEPROM",
            .value.get_value = get_eeprom_size,
            .is_dynamic = true
        }
    }
};

/**
 * Represents the memory information screen.
 */
const screen_content_t memory_info_screen = {
    .title = "Memory Info",
    .title_highlight = HIGHLIGHT_NONE,
    .elements = memory_info_elements,
    .element_count = 3,
    .highlight_index = 0,
    .default_x = 0,
    .default_y = 2,
    .center_contents = false,
    .get_highlight_index = NULL
};
