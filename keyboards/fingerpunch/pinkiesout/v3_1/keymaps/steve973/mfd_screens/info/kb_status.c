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
#include "modifiers.h"
#include "fp_pinkiesout.h"
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

static screen_element_t kb_status_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 2,
        .content.key_value = {
            .label = "Layer",
            .value.get_value = get_layer_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 3,
        .content.key_value = {
            .label = "Joystick",
            .value.get_value = get_joystick_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 4,
        .content.key_value = {
            .label = "Lock",
            .value.get_value = get_led_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 5,
        .content.key_value = {
            .label = "Mods",
            .value.get_value = get_mod_status,
            .is_dynamic = true
        }
    }
};

const screen_content_t kb_status_screen = {
    .title = "Keyboard Status",
    .elements = kb_status_elements,
    .element_count = sizeof(kb_status_elements) / sizeof(kb_status_elements[0]),
    .default_y = 2
};
