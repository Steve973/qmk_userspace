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

#include <stdlib.h>
#include <string.h>
#include "fp_joystick.h"
#include "deferred_exec.h"
#include "action_util.h"
#include "timer.h"
#include "wait.h"
#include "quantum/keycodes.h"

static void calibrate_max_min_values(void) {
    int16_t max_x = INT16_MIN, min_x = INT16_MAX;
    int16_t max_y = INT16_MIN, min_y = INT16_MAX;
    joystick_coordinate_t coordinates;
    uint32_t start_time;

    // Blink the "J" key to indicate joystick calibration
    blink_params_t blink_params = (blink_params_t){
        .cycle_duration_ms = 500,
        .keycode = KC_J,
        .target_rgb = (RGB){.r = 255, .g = 0, .b = 0},
        .pulse = true
    };

    // wait a maximum of 10 seconds for the user to start moving the joystick
    start_time = timer_read32();
    bool movement_started = false;
    while (!movement_started && timer_elapsed32(start_time) < 10000) {
        blink_key(blink_params, false);
        coordinates = apply_scaling_and_deadzone(read_joystick_coordinates());
        movement_started = coordinates.x_coordinate != 0 || coordinates.y_coordinate != 0;
        // Check for escape key to abort
        if (is_key_pressed(KC_ESC)) {
            blink_key(blink_params, true);
            return;
        }
        wait_ms(50);
    };

    start_time = timer_read32();
    while (timer_elapsed32(start_time) < CALIBRATION_DURATION_MS) {
        blink_key(blink_params, false);
        coordinates = apply_scaling_and_deadzone(read_joystick_coordinates());

        if (coordinates.x_coordinate > max_x) max_x = coordinates.x_coordinate;
        if (coordinates.x_coordinate < min_x) min_x = coordinates.x_coordinate;
        if (coordinates.y_coordinate > max_y) max_y = coordinates.y_coordinate;
        if (coordinates.y_coordinate < min_y) min_y = coordinates.y_coordinate;

        // Check for escape key to abort
        if (is_key_pressed(KC_ESC)) {
            blink_key(blink_params, true);
            return;
        }

        // Delay to control sampling rate
        wait_ms(js_profile.stick_timer_ms);
    }

    // Use max_x, min_x, max_y, min_y for calibration
    joystick_calibration.x_max = max_x;
    joystick_calibration.x_min = min_x;
    joystick_calibration.y_max = max_y;
    joystick_calibration.y_min = min_y;
}

/**
 * @brief Context for joystick calibration.
 */
struct calibration_context {
    int32_t total_x;
    int32_t total_y; 
    int16_t max_neutral_x;
    int16_t max_neutral_y;
    uint16_t sample_count;
    deferred_token token;
};

/**
 * @brief Finalizes the calibration of the joystick neutral position.
 *
 * This function calculates the neutral values for the joystick based on the input.
 *
 * @param cal The calibration context.
 */
static void finalize_neutral_calibration(struct calibration_context* cal) {
    int16_t ideal_neutral = (js_profile.raw_min + js_profile.raw_max) / 2;
    joystick_calibration.x_neutral = cal->total_x / CALIBRATION_SAMPLE_COUNT;
    joystick_calibration.y_neutral = cal->total_y / CALIBRATION_SAMPLE_COUNT;

    // Calculate scale_factor using fixed-point arithmetic
    uint16_t max_neutral = cal->max_neutral_x > cal->max_neutral_y ? cal->max_neutral_x : cal->max_neutral_y;
    joystick_calibration.scale_factor = (FIXED_POINT_SCALE * js_profile.out_max) / (js_profile.raw_max - max_neutral);

    int16_t x_drift = abs(joystick_calibration.x_neutral - ideal_neutral);
    int16_t y_drift = abs(joystick_calibration.y_neutral - ideal_neutral);
    int16_t min_deadzone_inner = x_drift > y_drift ? x_drift : y_drift;
    joystick_calibration.deadzone_inner = min_deadzone_inner > js_profile.deadzone_inner ? min_deadzone_inner : js_profile.deadzone_inner;
    joystick_calibration.deadzone_outer = js_profile.deadzone_outer;
}

/**
 * @brief Takes a calibration sample for the joystick neutral position.
 *
 * This function reads the joystick x and y values and updates the calibration context.
 *
 * @param current_time The current time.
 * @param context The calibration context.
 * @return The time to wait until the next sample.
 */
static uint32_t take_calibration_sample(uint32_t current_time, void* context) {
    struct calibration_context* cal = (struct calibration_context*)context;
    
    int16_t x = read_x_axis();
    int16_t y = read_y_axis();
    cal->total_x += x;
    cal->total_y += y;
    cal->max_neutral_x = MAX(x, cal->max_neutral_x);
    cal->max_neutral_y = MAX(y, cal->max_neutral_y);
    
    if (++cal->sample_count >= CALIBRATION_SAMPLE_COUNT) {
        finalize_neutral_calibration(cal);
        return 0;
    }
    
    return js_profile.stick_timer_ms;
}

/**
 * @brief Calibrates the joystick neutral position.
 *
 * This function updates the neutral values for the joystick based on the input.
 */
void calibrate_neutral_values(void) {
    static struct calibration_context cal;
    memset(&cal, 0, sizeof(cal));
    cal.token = defer_exec(js_profile.stick_timer_ms, take_calibration_sample, &cal);
}

void calibrate_range() {
    calibrate_max_min_values();
}
