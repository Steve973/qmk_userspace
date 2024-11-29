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

#include "fp_joystick.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"
#include "fingerpunch/src/fp_rgblight.h"
#include "quantum/keymap_common.h"
#include "quantum/keycodes.h"
#include "quantum/rgb_matrix/rgb_matrix.h"

/**
 * @brief Gets the LED index for the supplied keycode.
 *
 * This function obtains the LED index for the keycode supplied in the parameter.
 *
 * @param keycode the keycode of the key to get the corresponding LED index for.
 * @return The index of the LED at the key for the supplied keycode.
 */
uint8_t get_led_index(uint16_t keycode) {
    uint8_t index = NO_LED;
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            if (keymap_key_to_keycode(0, (keypos_t){col,row}) == keycode) {
                index = g_led_config.matrix_co[row][col];
                if (index >= 0 && index < RGB_MATRIX_LED_COUNT && index != NO_LED) break;
            }
        }
        if (index != NO_LED) break;
    }
    return index;
}

/*
 * @brief Blinks or pulses the key based on the provided parameters.
 *
 * This function blinks or pulses the key based on the provided parameters. Primarily
 * used to indicate the calibration process for the joystick.
 */
void blink_key(blink_params_t blink_params, bool reset) {
    static bool is_blinking = false;
    static uint32_t start_time;
    static uint8_t index = NO_LED;
    static HSV old_hsv;
    static uint8_t old_mode;
    static uint32_t color_value = 0;

    if (reset && is_blinking) {
        // Reset to original HSV and mode
        fp_rgb_set_hsv_and_mode(old_hsv.h, old_hsv.s, old_hsv.v, old_mode);
        is_blinking = false;
        return;
    }

    if (!is_blinking) {
        index = get_led_index(blink_params.keycode);
        if (index == NO_LED) return;
        old_hsv  = rgblight_get_hsv();
        old_mode = rgblight_get_mode();
        is_blinking = true;
        start_time = timer_read();
    }

    uint16_t elapsed = timer_elapsed(start_time);

    if (blink_params.pulse) {
        // Pulse between black and the configured target color
        uint16_t half_cycle = blink_params.cycle_duration_ms / 2;
        uint16_t phase = elapsed % blink_params.cycle_duration_ms;  // Keep as uint16_t

        uint32_t scaled_phase = phase < half_cycle ?
            // Fade in
            ((uint32_t)phase * 255) / half_cycle :
            // Fade out
            255 - (((uint32_t)(phase - half_cycle) * 255) / half_cycle);

        color_value = (uint8_t)scaled_phase;  // Constrain to 0-255

        rgb_matrix_set_color(index,
            (color_value * blink_params.target_rgb.r) >> 8,
            (color_value * blink_params.target_rgb.g) >> 8,
            (color_value * blink_params.target_rgb.b) >> 8
        );
    } else {
        // Simple blink between black and the configured target color
        if (elapsed < blink_params.cycle_duration_ms / 2) {
            rgb_matrix_set_color(index, 0, 0, 0);  // Black
        } else {
            rgb_matrix_set_color(index,
                blink_params.target_rgb.r,
                blink_params.target_rgb.g,
                blink_params.target_rgb.b);
        }
    }

    if (elapsed >= blink_params.cycle_duration_ms) {
        start_time = timer_read();
        elapsed = 0;
    }
}
