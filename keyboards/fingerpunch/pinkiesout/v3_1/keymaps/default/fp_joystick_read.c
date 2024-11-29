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
#include <math.h>
#include "analog.h"
#include "fp_joystick.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"

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
 * @brief Projects a value from one range to another.
 *
 * This function normalizes the input value from the source range and scales it
 * to the target range. It ensures that the scaled value is within the target range.
 *
 * @param val The value to be projected.
 * @param rmin The minimum value of the source range.
 * @param rmax The maximum value of the source range.
 * @param tmin The minimum value of the target range.
 * @param tmax The maximum value of the target range.
 * @return The projected value within the target range.
 */
static inline int16_t project(int16_t val, int16_t rmin, int16_t rmax, int16_t tmin, int16_t tmax) {
    // Check if the range is zero or negative
    if (abs(rmax - rmin) <= 0) return tmin;

    // Perform the projection calculation
    int32_t normalized = ((int32_t)(val - rmin) * 1024) / (rmax - rmin);
    int32_t scaled = (normalized * (tmax - tmin) + 512) / 1024 + tmin;

    return clamp(scaled, tmin, tmax);
}

/**
 * @brief Reads the x-axis value of the joystick.
 */
int16_t read_x_axis(void) {
    // return joystick_read_axis(0);
    return analogReadPin(VIK_GPIO_1);
}

/**
 * @brief Reads the y-axis value of the joystick.
 */
int16_t read_y_axis(void) {
    // return joystick_read_axis(1);
    return analogReadPin(VIK_GPIO_2);
}

/**
 * @brief Reads the analog joystick values.
 *
 * This function reads the analog joystick values from the configured pins.
 *
 * @return joystick coordinate type with x and y coordinates
 */
joystick_coordinate_t read_joystick_coordinates(void) {
    return (joystick_coordinate_t){
        read_x_axis(),
        read_y_axis()
    };
}

/**
 * @brief Applies scaling and deadzone to the joystick input.
 *
 * This function scales the raw joystick input values to the JOYSTICK_OUT_MIN to JOYSTICK_OUT_MAX range,
 * applies inner deadzone to the calibrated values, and assigns the scaled values to the output.
 *
 * @param rawx Raw x-axis value.
 * @param rawy Raw y-axis value.
 * @param outx Pointer to store the processed x-axis value.
 * @param outy Pointer to store the processed y-axis value.
 */
joystick_coordinate_t apply_scaling_and_deadzone(joystick_coordinate_t raw_coordinates) {
    // Convert to signed values centered at 0
    int16_t x = raw_coordinates.x_coordinate - joystick_calibration.x_neutral;
    int16_t y = raw_coordinates.y_coordinate - joystick_calibration.y_neutral;

    // Calculate squared distance from center (avoid sqrt)
    int32_t distance_sq = (int32_t)x * x + (int32_t)y * y;

    // Set values to zero if they are within the deadzone
    if (distance_sq < (int32_t)js_profile.deadzone_inner * js_profile.deadzone_inner) {
        x = y = 0;
    } else {
        // Apply the scaling to x and y using fixed-point arithmetic
        x = (x * joystick_calibration.scale_factor) / FIXED_POINT_SCALE;
        y = (y * joystick_calibration.scale_factor) / FIXED_POINT_SCALE;
    }

    return (joystick_coordinate_t){
        // Clamp to output range
        .x_coordinate = clamp(x, js_profile.out_min, js_profile.out_max),
        .y_coordinate = clamp(y, js_profile.out_min, js_profile.out_max)
    };
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
static joystick_coordinate_t handle_rotation(joystick_coordinate_t coordinates) {
    switch (joystick_config.up_orientation) {
        case JS_LEFT: // rotate 90 degrees counterclockwise
            return (joystick_coordinate_t){
                -coordinates.y_coordinate,
                coordinates.x_coordinate
            };
            break;
        case JS_DOWN: // rotate 180 degrees counterclockwise
            return (joystick_coordinate_t){
                -coordinates.x_coordinate,
                -coordinates.y_coordinate
            };
            break;
        case JS_RIGHT: // rotate 270 degrees counterclockwise
            return (joystick_coordinate_t){
                coordinates.y_coordinate,
                -coordinates.x_coordinate
            };
            break;
        case JS_UP: // up is up, so do not rotate
        default:
            return (joystick_coordinate_t){
                coordinates.x_coordinate,
                coordinates.y_coordinate
            };
            break;
    }
}

/**
 * @brief Get the raw (unaltered, un-rotated) direction based on joystick position.
 *
 * This function determines the direction based on the joystick position without any rotation.
 * It returns the direction number (0-3) based on the angle of the joystick.
 *
 * @return The direction number (0-3), or -1 if the joystick is at its neutral position.
 */
int8_t calculate_raw_direction(joystick_coordinate_t raw_coordinates) {
    joystick_coordinate_t coord = apply_scaling_and_deadzone(raw_coordinates);
    int16_t x = coord.x_coordinate;
    int16_t y = coord.y_coordinate;
    
    // Quick check for neutral
    if (x == 0 && y == 0) return -1;

    // Use absolute values for comparison
    int16_t abs_x = abs(x);
    int16_t abs_y = abs(y);
    
    // Check if more horizontal or vertical
    if (abs_x > abs_y) {
        // Horizontal movement
        return x > 0 ? JS_RIGHT : JS_LEFT;
    } else if (abs_y > abs_x) {
        // Vertical movement
        return y > 0 ? JS_UP : JS_DOWN;
    }
    
    return -1;  // Diagonal
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
    joystick_coordinate_t coordinates = read_joystick_coordinates();
    int8_t direction = calculate_raw_direction(coordinates);
    if (rotate && direction > -1) {
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
 * @brief Reads the joystick input.
 *
 * This function reads the raw joystick input values, processes
 * deadzones, scales the values, and applies rotation based on
 * the current configuration.
 *
 * @return the coordinates that were read from the joystick.
 */
joystick_coordinate_t read_joystick(void) {
    joystick_coordinate_t coordinates = read_joystick_coordinates();
    coordinates = apply_scaling_and_deadzone(coordinates);
    coordinates = handle_rotation(coordinates);
    return coordinates;
}
