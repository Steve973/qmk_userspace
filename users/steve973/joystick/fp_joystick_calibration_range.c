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

#include <math.h>
#include <string.h>
#include <time.h>
#include "_wait.h"
#include "deferred_exec.h"
#include "fp_joystick.h"

/**
 * @brief Context for joystick range calibration.
 */
struct range_cal_context {
    int32_t min_edge_distance_sq;
    uint8_t sample_count;
};

/**
 * @brief Finalizes the calibration of the joystick range.
 *
 * This function calculates the range values for the joystick based on the input.
 *
 * @param cal The calibration context.
 */
static void finalize_range_calibration(struct range_cal_context* cal) {
    joystick_calibration.deadzone_outer_percent =
        MIN((sqrtl(cal->min_edge_distance_sq) * 95) / js_profile_out.max,
            joystick_calibration.deadzone_outer_percent);

    fp_kb_config_user.js_calibration = joystick_calibration;
    joystick_calibration.last_range_cal = time(NULL) / ONE_DAY;
    fp_kb_config_save();
}

/**
 * @brief Take a sample of the joystick values and process it.
 *
 * Takes a sample of the joystick and processes the coordinate to normalize
 * the values for zero-neutral, and scale the coordinates to the range that is
 * specified in the joystick_profile_out values. The sample is checked to
 * see that it is at least 85% of the maximum range, and if so, it checks
 * this value against the current minimum edge distance squared.
 *
 * @param cal The calibration context to update.
 */
static void process_range_cal_sample(struct range_cal_context* cal) {
    int16_t x = read_x_axis();
    int16_t y = read_y_axis();
    normalize_joystick_coordinates(&x, &y);
    scale_joystick_coordinates(&x, &y);

    int32_t distance_sq = (int32_t)x * x + (int32_t)y * y;
    if (distance_sq > (js_profile_out.max * js_profile_out.max * 85) / 100) {
        cal->min_edge_distance_sq = MIN(cal->min_edge_distance_sq, distance_sq);
        ++cal->sample_count;
    }
}

/**
 * @brief Takes a calibration sample for the joystick range position asynchronously.
 *
 * This function reads the joystick x and y values and updates the calibration context.
 *
 * @param current_time The current time.
 * @param context The calibration context.
 * @return The time to wait until the next sample.
 */
static uint32_t range_cal_handler(uint32_t current_time, void* context) {
    struct range_cal_context* cal = (struct range_cal_context*)context;
    process_range_cal_sample(cal);

    if (cal->sample_count >= CALIBRATION_SAMPLE_COUNT) {
        finalize_range_calibration(cal);
        return 0;
    }
    return joystick_config.read_interval_ms;
}

/**
 * @brief Calibrates the joystick range.
 *
 * This function updates the outer deadzone for the joystick based on the input.
 * For invocation, you have the option to run the calibration asynchronously,
 * or synchronously, depending on the value of the `async` parameter.
 *
 * @param async Whether to calibrate asynchronously (true) or synchronously (false).
 */
void calibrate_range(bool async) {
    static struct range_cal_context cal;  // Static for async persistence
    memset(&cal, 0, sizeof(cal));
    cal.min_edge_distance_sq = INT32_MAX;

    if (async) {
        defer_exec(joystick_config.read_interval_ms, range_cal_handler, &cal);
    } else {
        while (cal.sample_count < CALIBRATION_SAMPLE_COUNT) {
            process_range_cal_sample(&cal);
            wait_ms(joystick_config.read_interval_ms);
        }
        finalize_range_calibration(&cal);
    }
}
