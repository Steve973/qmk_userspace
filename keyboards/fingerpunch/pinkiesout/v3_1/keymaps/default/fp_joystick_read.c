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
 * @brief Clamps a value between a minimum and maximum range.
 *
 * This function ensures that the input value is within the specified range.
 * If the value is less than the minimum, it returns the minimum.
 * If the value is greater than the maximum, it returns the maximum.
 * Otherwise, it returns the value itself.
 *
 * @param val The value to be clamped.
 * @param min The minimum allowable value.
 * @param max The maximum allowable value.
 * @return The clamped value.
 */
static inline int16_t clamp(int16_t val, int16_t min, int16_t max) {
    return (val < min) ? min : ((val > max) ? max : val);
}

/**
 * @brief Reads the x-axis value of the joystick.
 */
int16_t read_x_axis(void) {
    return analogReadPin(VIK_GPIO_1);
}

/**
 * @brief Reads the y-axis value of the joystick.
 */
int16_t read_y_axis(void) {
    return analogReadPin(VIK_GPIO_2);
}

/**
 * @brief Applies rotation to the joystick values based on the current configuration.
 *
 * This function rotates the joystick values according to the configured up orientation value.
 * For example, if the joystick is installed where it is rotated a quarter turn
 * counterclockwise, like this:
 *
 *      R
 *      ↑
 * U ←  ●  → D
 *      ↓
 *      L
 *
 * Then the "up orientation" property of the joystick configuration would be "RIGHT", and
 * the joystick coordinates would have to be rotated 270 degrees counterclockwise.
 *
 * @param coordinates The coordinates that were read from the joystick.
 * @return The coordinates rotated based on which electrical direction is installed in
 * the physical "up" orientation.
 */
void handle_rotation(int16_t* x, int16_t* y) {
    int16_t tmp_x = *x;
    switch (joystick_config.up_orientation) {
        case JS_LEFT: // rotate 90 degrees clockwise
            *x = *y;
            *y = -tmp_x;
            return;
        case JS_DOWN: // rotate 180 degrees
            *x = -(*x);
            *y = -(*y);
            return;
        case JS_RIGHT: // rotate 270 degrees clockwise
            *x = -(*y);
            *y = tmp_x;
            return;
        default: // up is up, so do not rotate
            return;
    }
}

/**
 * @brief Applies deadzone processing to the joystick values.
 * 
 * If the joystick is within the inner deadzone, the x and y values are set to zero.
 * If the joystick is within the outer deadzone, the x and y values are boosted to
 * ensure that they can reach the maximum value.
 */
void apply_deadzones(int16_t* x, int16_t* y) {
    int32_t distance_sq = (int32_t)(*x) * (*x) + (int32_t)(*y) * (*y);
    int32_t max_distance_sq = (int32_t)js_profile_out.max * js_profile_out.max;

    if ((distance_sq * 100) < (max_distance_sq * joystick_calibration.deadzone_inner_percent)) {
        *x = *y = 0;
    } else if ((distance_sq * 100) > (max_distance_sq * joystick_calibration.deadzone_outer_percent)) {
        // Apply boost at max range
        *x += *x / 8;  // +12.5%
        *y += *y / 8;
    }
}

/**
 * @brief Scales the joystick values based on the configured shift factor.
 */
void scale_joystick_coordinates(int16_t* x, int16_t* y) {
    *x = *x >> joystick_calibration.shift_factor;
    *y = *y >> joystick_calibration.shift_factor;
}

/**
 * @brief Normalizes the joystick coordinates based on the raw neutral value.
 */
void normalize_joystick_coordinates(int16_t* x, int16_t* y) {
    *x = *x - js_profile_raw.neutral;
    *y = *y - js_profile_raw.neutral;
}

/**
 * @brief Get the raw (unaltered, un-rotated) direction based on joystick position.
 *
 * This function determines the direction based on the joystick position without any rotation.
 * It returns the direction number (0-3) based on the angle of the joystick.
 *
 * @return The direction number (0-3), or -1 if the joystick is at its neutral position.
 */
static int8_t calculate_raw_direction(void) {
    int16_t x = read_x_axis();
    int16_t y = read_y_axis();
    dprintf("Raw x,y: %d,%d\n", x, y);
    normalize_joystick_coordinates(&x, &y);
    dprintf("Normalized x,y: %d,%d\n", x, y);
    scale_joystick_coordinates(&x, &y);
    dprintf("Scaled x,y: %d,%d\n", x, y);
    apply_deadzones(&x, &y);
    dprintf("After deadzone x,y: %d,%d\n", x, y);
    
    int8_t direction;
    if (x == 0 && y == 0) {
        direction = JS_NEUTRAL;
    } else if (abs(x) > abs(y)) {
        direction = x > 0 ? JS_RIGHT : JS_LEFT;
    } else if (abs(y) > abs(x)) {
        direction = y > 0 ? JS_UP : JS_DOWN;
    } else {
        direction = JS_NEUTRAL;
    }
    
    dprintf("Raw direction: %d\n", direction);
    return direction;
}

/**
 * @brief Get the direction based on joystick position.
 *
 * This function determines the direction based on the joystick position, and then.
 * returns the direction number (0-3) based on the angle of the joystick.  If "rotate"
 * is true, then it rotates the direction based on the physically installed orientation
 * of the joystick.
 *
 * For example, if the joystick is installed where it is rotated a quarter turn
 * counterclockwise, like this:
 *
 *      R
 *      ↑
 * U ←  ●  → D
 *      ↓
 *      L
 *
 * Then, if the joystick is pointing up, the direction would be reported as JS_RIGHT.
 * If the "rotate" parameter is true, the coordinates would have to be rotated to ensure
 * that the returned value is JS_UP to correspond with the movement the user expects.
 *
 * @param out_direction a pointer to return the calculated direction to the caller
 * @param out_angle a pointer to return the calculated angle to the caller
 * @param rotate a boolean flag to indicate if the caller wants the joystick rotated
 *               based on the configured "up orientation" of the joystick
 * @return The direction number (0-3), or -1 if the joystick is at its neutral position.
 */
int8_t calculate_direction(bool rotate) {
    int8_t direction = calculate_raw_direction();
    if (rotate && direction != JS_NEUTRAL) {
        switch (joystick_config.up_orientation) {
            case JS_LEFT:
                direction = (direction + 3) % ORIENTATION_COUNT;
                break;
            case JS_DOWN:
                direction = (direction + 2) % ORIENTATION_COUNT;
                break;
            case JS_RIGHT:
                direction = (direction + 1) % ORIENTATION_COUNT;
                break;
            case JS_UP:
            default:
                break;
       }
    }
    return direction;
}

/**
 * @brief Waits for joystick movement.
 *
 * This function waits for the joystick to move a certain percentage of its maximum range
 * within a specified timeout period.  If the joystick moves more than the specified percentage
 * of its maximum range, the function returns true.  If the joystick does not move within the
 * specified timeout, or the specified percentage of its maximum range, the function returns false.
 *
 * @param timeout_ms The maximum time to wait for joystick movement.
 * @param percent The percentage of the maximum range that the joystick must move to return true.
 * @return true if the joystick moves the specified percentage of its maximum range, false otherwise.
 */
bool wait_for_js_movement(uint32_t timeout_ms, uint8_t percent) {
    int16_t x, y;
    uint32_t start_time = timer_read32();
    while (timer_elapsed32(start_time) < timeout_ms) {
        if (is_key_pressed(KC_ESC)) return false;
        x = read_x_axis();
        y = read_y_axis();
        normalize_joystick_coordinates(&x, &y);
        scale_joystick_coordinates(&x, &y);
        if (((int32_t)x * x + (int32_t)y * y) > ((js_profile_out.max * js_profile_out.max * percent) / 100)) {
            return true;
        }
        wait_ms(joystick_config.read_interval_ms);
    }
    return false;
}

/**
 * @brief Reads the joystick input.
 *
 * This function reads the raw joystick input values, processes
 * deadzones, scales the values, and applies rotation based on
 * the current configuration.
 *
 * @return the coordinates that were read from the joystick.
 */
joystick_coordinate_t read_joystick(void) {
    int16_t x = read_x_axis();
    int16_t y = read_y_axis();
    normalize_joystick_coordinates(&x, &y);
    scale_joystick_coordinates(&x, &y);
    apply_deadzones(&x, &y);
    if (x != 0 || y != 0) {
        handle_rotation(&x, &y);
    }
    return (joystick_coordinate_t){
        .x_coordinate = clamp(x, js_profile_out.min, js_profile_out.max),
        .y_coordinate = clamp(y, js_profile_out.min, js_profile_out.max)
    };
}
