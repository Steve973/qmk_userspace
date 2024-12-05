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

/**
 * @brief Context for joystick calibration.
 */
struct neutral_cal_context {
    int16_t max_neutral_drift;
    uint8_t sample_count;
};

/**
 * @brief Finalizes the calibration of the joystick neutral position.
 *
 * This function calculates the neutral values for the joystick based on the input.
 *
 * @param cal The calibration context.
 */
static void finalize_neutral_calibration(struct neutral_cal_context* cal) {
    uint8_t inner_drift_percent = (cal->max_neutral_drift * 100) / js_profile_out.max;
    
    // Sanity check - fail if drift is excessive
    if (inner_drift_percent > 20) {
        dprintf("Warning: Excessive neutral drift detected: %d%%\n", inner_drift_percent);
    }

    joystick_calibration.deadzone_inner_percent = MAX(inner_drift_percent, joystick_calibration.deadzone_inner_percent);
    dprintf("Neutral calibration complete. Drift: %d%%, Deadzone: %d%%\n", 
            inner_drift_percent, joystick_calibration.deadzone_inner_percent);
    
    fp_kb_config_user.js_calibration = joystick_calibration;
    joystick_calibration.last_neutral_cal = time(NULL) / ONE_DAY;
    fp_kb_config_save();
}

/**
 * @brief Take a sample of the joystick values and process it.
 * 
 * Takes a sample of the joystic and processes the coordinate to normalize
 * the values for zero-neutral, and scale the coordinates to the range that
 * is specified in the joystick_profile_out values.
 * 
 * @param cal The calibration context to update.
 */
static void process_neutral_cal_sample(struct neutral_cal_context* cal) {
    int16_t x = abs(read_x_axis());
    int16_t y = abs(read_y_axis());
    normalize_joystick_coordinates(&x, &y);
    scale_joystick_coordinates(&x, &y);
    cal->max_neutral_drift = MAX(cal->max_neutral_drift, MAX(x, y));
    ++cal->sample_count;
}

/**
 * @brief Takes a calibration sample for the joystick neutral position asynchronously.
 *
 * This function reads the joystick x and y values and updates the calibration context.
 *
 * @param current_time The current time.
 * @param context The calibration context.
 * @return The time to wait until the next sample.
 */
static uint32_t neutral_cal_handler(uint32_t current_time, void* context) {
    struct neutral_cal_context* cal = (struct neutral_cal_context*)context;
    process_neutral_cal_sample(cal);
    
    if (cal->sample_count >= CALIBRATION_SAMPLE_COUNT) {
        finalize_neutral_calibration(cal);
        return 0;
    }
    return joystick_config.read_interval_ms;
}

/**
 * @brief Calibrates the joystick neutral position.
 *
 * This function updates the neutral values for the joystick based on the input.
 * For invocation, you have the option to run the calibration asynchronously,
 * or synchronously, depending on the value of the `async` parameter.
 */
void calibrate_neutral_values(bool async) {
    static struct neutral_cal_context cal;  // Static for async persistence
    memset(&cal, 0, sizeof(cal));

    if (async) {
        defer_exec(joystick_config.read_interval_ms, neutral_cal_handler, &cal);
    } else {
        while (cal.sample_count < CALIBRATION_SAMPLE_COUNT) {
            process_neutral_cal_sample(&cal);
            wait_ms(joystick_config.read_interval_ms);
        }
        finalize_neutral_calibration(&cal);
    }
}