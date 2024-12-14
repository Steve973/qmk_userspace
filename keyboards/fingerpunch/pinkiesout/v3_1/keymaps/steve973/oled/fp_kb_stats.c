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
#include "oled/oled_driver.h"
#include "quantum/led.h"
#include "progmem.h"
#include "tmk_core/protocol/host.h"
#include "quantum/rgb_matrix/rgb_matrix.h"
#include "wpm.h"
#include "modifiers.h"
#include "../fp_pinkiesout.h"
#include "joystick/fp_joystick.h"

void render_layer_status(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state|default_layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("Qwerty\n"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("Lower\n"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("Raise\n"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("Adjust\n"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined\n"), false);
    }
}

void render_joystick_status(void) {
    #ifdef JOYSTICK_ENABLE
    oled_write_P(PSTR("Joystick: "), false);
    switch (get_stick_mode()) {
        case JOYSTICK_SM_ARROWS:
            oled_write_P(PSTR("Arrows\n"), false);
            break;
        case JOYSTICK_SM_WASD:
            oled_write_P(PSTR("WASD\n"), false);
            break;
        case JOYSTICK_SM_ANALOG:
            oled_write_P(PSTR("Analog\n"), false);
            break;
        case JOYSTICK_SM_MOUSE:
            oled_write_P(PSTR("Mouse\n"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined\n"), false);
    }
    #endif // JOYSTICK_ENABLE
}

void render_kb_led_status(void) {
    // Write host Keyboard LED Status to OLED
    led_t led_usb_state = host_keyboard_led_state();
    char lock_status[13] = {0};
    oled_write_P(PSTR("Lock: "), false);
    if (led_usb_state.caps_lock || led_usb_state.num_lock || led_usb_state.scroll_lock) {
        strcat(lock_status, led_usb_state.caps_lock ? PSTR("CAP ") : PSTR("    "));
        strcat(lock_status, led_usb_state.num_lock ? PSTR("NUM ") : PSTR("    "));
        strcat(lock_status, led_usb_state.scroll_lock ? PSTR("SCR") : PSTR("    "));
        oled_write(lock_status, false);
    } else {
        oled_write_P(PSTR("None           "), false);
    }
}

void render_mod_status(void) {
    // Write host Keyboard Mod Status to OLED
    uint8_t mods = get_mods();
    char mod_status[16] = {0};
    oled_write_P(PSTR("Mods: "), false);
    if (mods != 0) {
        strcat(mod_status, mods & MOD_MASK_SHIFT ? PSTR("SHF ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_CTRL ? PSTR("CTL ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_ALT ? PSTR("ALT ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_GUI ? PSTR("GUI") : PSTR("   "));
        oled_write(mod_status, false);
    } else {
        oled_write_P(PSTR("None           "), false);
    }
}

void render_wpm(void) {
    static uint8_t peak_wpm = 0;
    char wpm_str[15];
    uint8_t current = get_current_wpm();
    if (current > peak_wpm) {
        peak_wpm = current;
    }
    sprintf(wpm_str, "WPM: %3d (%3d)", current, peak_wpm);
    oled_write_P(PSTR(wpm_str), false);
}

void render_uptime(void) {
    uint32_t elapsed = timer_read32() / 1000;
    char uptime_str[17];
    sprintf(uptime_str, "Uptime: %02lu:%02lu", elapsed/60, elapsed%60);
    oled_write_P(PSTR(uptime_str), false);
}

void render_keycount(void) {
    char count_str[18];
    sprintf(count_str, "Keycount: %lu", keypress_count);
    oled_write_P(PSTR(count_str), false);
}

void render_rgb_status(void) {
    #ifdef RGB_MATRIX_ENABLE
    if (rgb_matrix_is_enabled()) {
        char rgb_str[22];
        sprintf(rgb_str, "HSV:%3d,%3d,%3d M:%d",
            rgb_matrix_get_hue(),
            rgb_matrix_get_sat(),
            rgb_matrix_get_val(),
            rgb_matrix_get_mode());
        oled_write_P(PSTR(rgb_str), false);
    } else {
        oled_write_P(PSTR("RGB: Off             "), false);
    }
    #endif
}

void render_key_rates(void) {
    static uint32_t last_time = 0;
    static uint32_t last_keycount = 0;
    static uint16_t keys_per_sec = 0;
    static uint16_t keys_per_min = 0;

    uint32_t now = timer_read32();
    uint32_t elapsed = now - last_time;

    // Update every second
    if (elapsed >= 1000) {
        keys_per_sec = keypress_count - last_keycount;
        keys_per_min = keys_per_sec * 60;  // per minute
        last_keycount = keypress_count;
        last_time = now;
    }

    char rate_str[20];
    sprintf(rate_str, "KPS:%2d KPM:%4d", keys_per_sec, keys_per_min);
    oled_write_P(PSTR(rate_str), false);
}

void oled_display_stats(void) {
    oled_set_cursor(0, 1);
    render_layer_status();
    oled_set_cursor(0, 2);
    render_joystick_status();
    oled_set_cursor(0, 3);
    render_rgb_status();
    oled_set_cursor(0, 4);
    render_key_rates();
    oled_set_cursor(0, 5);
    render_wpm();
    oled_set_cursor(0, 6);
    render_keycount();
    oled_set_cursor(0, 7);
    render_mod_status();
    oled_set_cursor(0, 8);
    render_kb_led_status();
    oled_set_cursor(0, 9);
    render_uptime();
}
