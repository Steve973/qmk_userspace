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

#include QMK_KEYBOARD_H
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "action_util.h"
#include "analog.h"
#include "color.h"
#include "gpio.h"
#include "joystick.h"
#include "keycodes.h"
#include "pin_defs.h"
#include "pico/types.h"
#include "rgb_matrix/rgb_matrix_types.h"
#include "rgb_matrix/rgb_matrix.h"
#include "src/fp_rgb_common.h"
#include "fp_joystick.h"

joystick_profile_t js_profile = JOYSTICK_PROFILE;

typedef void (*stick_mode_handler)(int8_t x, int8_t y);

static uint32_t stick_timer;
static joystick_calibration_t joystick_calibration;
static fp_joystick_config_t joystick_config;

static void handle_analog(int8_t x, int8_t y);
static void handle_wasd(int8_t x, int8_t y);
static void handle_arrows(int8_t x, int8_t y);

/**
 * Divides the circle into 24 segments for easy, if less precise,
 * angle calculations.  Each point is 15 degrees apart to avoid
 * floating-point math for AVR processors that do not have a native
 * FPU.
 */
static const stick_direction_t angle_to_direction[ANGLE_DIVISIONS] = {
    // Right / East / Quadrant I
    DIR_E, DIR_E15, DIR_E30, DIR_E45, DIR_E60, DIR_E75,
    // Up / North / Quadrant II
    DIR_N, DIR_N15, DIR_N30, DIR_N45, DIR_N60, DIR_N75,
    // Left / West / Quadrant III
    DIR_W, DIR_W15, DIR_W30, DIR_W45, DIR_W60, DIR_W75,
    // Down / South / Quadrant IV
    DIR_S, DIR_S15, DIR_S30, DIR_S45, DIR_S, DIR_S75
};

/**
 * Maps the axis angle value to the ordinal value of the
 * direction.
 */
static const struct {
    axis_directions axis_direction;
    joystick_up_orientation orientation;
} directions[] = {
    {AXIS_RIGHT, JS_RIGHT},
    {AXIS_UP,    JS_UP},
    {AXIS_LEFT,  JS_LEFT},
    {AXIS_DOWN,  JS_DOWN}
};

fp_joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT];

static void setup_joystick_axes(void) {
    size_t num_pins = sizeof(js_profile.axis_pins) / sizeof(js_profile.axis_pins[0]);
    for (int i = 0; i < num_pins; i++) {
        pin_t axis_pin = js_profile.axis_pins[i];
        joystick_axes[i] = axis_pin != NO_PIN ?
            (fp_joystick_config_t) JOYSTICK_AXIS_IN(axis_pin, js_profile.raw_min, js_profile.raw_neutral, js_profile.raw_max) :
            (fp_joystick_config_t) JOYSTICK_AXIS_VIRTUAL;
    }
}

/**
 * @brief Reads the x-axis value of the joystick.
 */
static int16_t read_x_axis(void) {
    return joystick_read_axis(0);
}

/**
 * @brief Reads the y-axis value of the joystick.
 */
static int16_t read_y_axis(void) {
    return joystick_read_axis(1);
}

/**
 * @brief Reads the analog joystick values.
 *
 * This function reads the analog joystick values from the configured pins.
 *
 * @return joystick coordinate type with x and y coordinates
 */
static joystick_coordinate_t read_joystick_coordinates(void) {
    return (joystick_coordinate_t){
        read_x_axis(),
        read_y_axis()
    };
}

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

void blink_key(blink_params_t blink_params, bool reset) {
    static bool is_blinking = false;
    static uint16_t start_time = 0;
    static uint8_t index = KC_NO;
    static HSV old_hsv;
    static uint8_t old_mode;

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

    if (elapsed >= blink_params.cycle_duration_ms) {
        start_time = timer_read();
        elapsed = 0;
    }

    if (blink_params.pulse) {
        // Pulse between black and the configured target color
        uint8_t color_value;
        if (elapsed < blink_params.cycle_duration_ms / 2) {
            color_value = (elapsed * 255) / (blink_params.cycle_duration_ms / 2);
        } else {
            color_value = 255 - ((elapsed - blink_params.cycle_duration_ms / 2) * 255) / (blink_params.cycle_duration_ms / 2);
        }

        rgb_matrix_set_color(index,
            (color_value * blink_params.target_rgb.r) / 255,
            (color_value * blink_params.target_rgb.g) / 255,
            (color_value * blink_params.target_rgb.b) / 255
        );
    } else {
        // Simple blink between black and the configured target color
        if (elapsed < blink_params.cycle_duration_ms / 2) {
            rgb_matrix_set_color(index, 0, 0, 0);  // Black
        } else {
            rgb_matrix_set_color(index,
                blink_params.target_rgb.r,
                blink_params.target_rgb.g,
                blink_params.target_rgb.b
            );
        }
    }
}

/**
 * @brief Checks if the joystick movement exceeds the deadzone, or sets the values to zero/neutral.
 *
 * This function checks joystick input values to determine if they are moved past the deadzone.
 * If not, the values are set to zero/neutral.  Otherwise, they are unaltered, and returned as
 * a new instance of coordinates.
 *
 * @param coordinates The coordinates read from the joystick.
 * @return the joystick coordinates that were read, if they exceed the deadzone, or with neutral
 * coordinates if the read values were within the deadzone.
 */
static joystick_coordinate_t check_deadzone(joystick_coordinate_t coordinates) {
    int16_t x = coordinates.x_coordinate;
    int16_t y = coordinates.y_coordinate;

    // Calculate squared distance from center (avoid sqrt)
    int32_t distance_sq = (int32_t)x * x + (int32_t)y * y;

    // Set values to zero if they are within the deadzone
    if (distance_sq < (int32_t)js_profile.deadzone_inner * js_profile.deadzone_inner) {
        x = y = 0;
    }

    return (joystick_coordinate_t){x, y};
}

static void calibrate_max_min_values(joystick_calibration_t* stick) {
    int16_t max_x = INT16_MIN, min_x = INT16_MAX;
    int16_t max_y = INT16_MIN, min_y = INT16_MAX;
    joystick_coordinate_t coordinates;
    uint32_t start_time;

    // Blink the "J" key to indicate joystick calibration
    blink_params_t blink_params = (blink_params_t){
        .cycle_duration_ms = 500,
        .keycode = KC_J,
        .pulse = true
    };
    blink_key(blink_params, false);

    // wait a maximum of 10 seconds for the user to start moving the joystick
    start_time = timer_read32();
    bool movement_started = false;
    while (!movement_started && timer_elapsed32(start_time) < 10000) {
        coordinates = check_deadzone(read_joystick_coordinates());
        movement_started = coordinates.x_coordinate != 0 || coordinates.y_coordinate != 0;
        // Check for escape key to abort
        if (is_key_pressed(KC_ESC)) {
            blink_key(blink_params, true);
            return;
        }
        wait_ms(50);
    } while (!movement_started);

    start_time = timer_read32();
    while (timer_elapsed32(start_time) < CALIBRATION_DURATION_MS) {
        coordinates = check_deadzone(read_joystick_coordinates());

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
    stick->x_max = max_x;
    stick->x_min = min_x;
    stick->y_max = max_y;
    stick->y_min = min_y;
}

/**
 * @brief Calibrates the joystick neutral position.
 *
 * This function updates the neutral values for the joystick based on the input.
 *
 * @param stick Pointer to the joystick calibration data.
 */
static void calibrate_neutral_values(joystick_calibration_t* stick) {
    int16_t ideal_neutral = (js_profile.raw_min + js_profile.raw_max) / 2;
    int32_t total_x = 0, total_y = 0;
    int16_t x = 0, y = 0, max_neutral_x = 0, max_neutral_y = 0;

    for (int i = 0; i < CALIBRATION_SAMPLE_COUNT; i++) {
        x = read_x_axis();
        y = read_y_axis();
        total_x += x;
        total_y += y;
        max_neutral_x = x > max_neutral_x ? x : max_neutral_x;
        max_neutral_y = y > max_neutral_y ? y : max_neutral_y;
        wait_ms(js_profile.stick_timer_ms);
    }

    stick->x_neutral = total_x / CALIBRATION_SAMPLE_COUNT;
    stick->y_neutral = total_y / CALIBRATION_SAMPLE_COUNT;

    int16_t x_drift = abs(stick->x_neutral - ideal_neutral);
    int16_t y_drift = abs(stick->y_neutral - ideal_neutral);
    int16_t min_deadzone_inner = x_drift > y_drift ? x_drift : y_drift;
    stick->deadzone_inner = min_deadzone_inner > js_profile.deadzone_inner ? min_deadzone_inner : js_profile.deadzone_inner;
    stick->deadzone_outer = js_profile.deadzone_outer;
}

void calibrate_range(void) {
    calibrate_max_min_values(&joystick_calibration);
}

/**
 * @brief Array of joystick mode handlers.
 *
 * This constant array defines the different handling modes for the joystick.
 * Each mode corresponds to a specific function that processes the joystick input
 * in a different way. The available modes are:
 * - `handle_analog`: Handles analog joystick input.
 * - `handle_wasd`: Maps joystick input to WASD keys.
 * - `handle_arrows`: Maps joystick input to arrow keys.
 */
static const stick_mode_handler stick_modes[] = {
    handle_analog,
    handle_wasd,
    handle_arrows
};

/**
 *
 * @brief Handles the registration and unregistration of keys based on joystick axis movement.
 *
 * This function determines the current and previous states of the joystick axis and registers
 * or unregisters the corresponding keys based on the actuation point.
 *
 * @param curr The current axis value.
 * @param prev The previous axis value.
 * @param pos_key The key to register when the axis value is positive.
 * @param neg_key The key to register when the axis value is negative.
 */
static void handle_axis(int8_t curr, int8_t prev, uint16_t pos_key, uint16_t neg_key) {
    int8_t curr_state = (curr > js_profile.actuation_point) - (curr < -js_profile.actuation_point);
    int8_t prev_state = (prev > js_profile.actuation_point) - (prev < -js_profile.actuation_point);
    bool should_register = (curr_state != 0);
    if (curr_state != prev_state) {
        uint16_t key_to_handle = (should_register) ?
            (curr_state > 0 ? pos_key : neg_key) :
            (prev_state > 0 ? pos_key : neg_key);
        if (should_register) {
            register_code16(key_to_handle);
            dprintf("registering %d\n", key_to_handle);
        } else {
            unregister_code16(key_to_handle);
            dprintf("unregistering %d\n", key_to_handle);
        }
    }
}

/**
 * @brief Handles the registration and unregistration of four keys based on joystick movement.
 *
 * This function processes the joystick x and y values and registers or unregisters
 * the corresponding keys (up, left, down, right) based on the actuation point.
 *
 * @param x The x-axis value.
 * @param y The y-axis value.
 * @param u The key to register when the y-axis value is positive (up).
 * @param l The key to register when the x-axis value is negative (left).
 * @param d The key to register when the y-axis value is negative (down).
 * @param r The key to register when the x-axis value is positive (right).
 */
static void handle_joystick_keys_4(int8_t x, int8_t y, uint16_t u, uint16_t l, uint16_t d, uint16_t r) {
    static int8_t px, py;
    handle_axis(y, py, u, d);
    handle_axis(x, px, r, l);
    px = x;
    py = y;
}

/**
 * @brief Handles analog joystick input.
 *
 * This function sets the joystick axis values based on the input x and y values.
 *
 * @param x The x-axis value.
 * @param y The y-axis value.
 */
static void handle_analog(int8_t x, int8_t y) {
    joystick_set_axis(0, x);
    joystick_set_axis(1, y);
}

/**
 * @brief Handles WASD key input based on joystick movement.
 *
 * This function maps the joystick x and y values to the corresponding WASD keys.
 *
 * @param x The x-axis value.
 * @param y The y-axis value.
 */
static void handle_wasd(int8_t x, int8_t y) {
    handle_joystick_keys_4(x, y, KC_W, KC_A, KC_S, KC_D);
}

/**
 * @brief Handles arrow key input based on joystick movement.
 *
 * This function maps the joystick x and y values to the corresponding arrow keys.
 * It also prints a message every second to indicate that it is handling arrow keys.
 *
 * @param x The x-axis value.
 * @param y The y-axis value.
 */
static void handle_arrows(int8_t x, int8_t y) {
    handle_joystick_keys_4(x, y, KC_UP, KC_LEFT, KC_DOWN, KC_RGHT);
}

/**
 * @brief Gets the orientation of the joystick's direction facing "up" from the perspective
 * of the user, based on how it is installed.
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
 * the installed orientation has the "RIGHT" direction facing up.  So, the value for
 * the "up orientation", in this case, is "RIGHT".
 *
 * @return The current up orientation value.
 */
int8_t get_stick_up_orientation() {
    return joystick_config.up_orientation;
}

/**
 * @brief Sets the orientation of the joystick's direction facing "up" from the perspective
 * of the user, based on how it is installed.
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
 * the installed orientation has the "RIGHT" direction facing up.  So, the value for
 * the "up orientation", in this case, is "RIGHT".
 *
 * @param up_orientation The new stick up orientation value.
 */
void set_stick_up_orientation(joystick_up_orientation up_orientation) {
    joystick_config.up_orientation = up_orientation;
}

/**
 * @brief Cycles through the joystick modes.
 *
 * This function sets the joystick mode, resets the joystick axes and prints
 * the new mode.
 */
void set_stick_mode(uint8_t mode) {
    joystick_set_axis(0, 0);
    joystick_set_axis(1, 0);
    joystick_config.mode = mode;
    dprintf("Stick mode now %i\n", joystick_config.mode);
}

/**
 * @brief Cycles through the joystick modes.
 *
 * This function increments the joystick mode and wraps around if it exceeds the
 * maximum mode value.
 */
void step_stick_mode(void) {
    set_stick_mode((joystick_config.mode + 1) % JOYSTICK_SM_END);
}

/**
 * @brief Gets the angle of the joystick's "up" direction based on how it is installed.
 *
 * This function returns the current angle of the joystick's "up" direction based on how it is installed.
 * For example, if the joystick is installed where it is rotated a quarter turn
 * counterclockwise, like this:
 *
 *      R
 *      ↑
 * U ←  ●  → D
 *      ↓
 *      L
 *
 * the value would be 0 degrees.
 *
 * @return The current up angle value.
 */
static int16_t get_stick_up_angle(void) {
    return joystick_config.up_orientation * 90;
}

/**
 * @brief Calculates the angle based on the joystick position.
 *
 * This function calculates the angle of the joystick position
 * in degrees, ranging from 0 to 360.  This is the "trig" implementation
 * that uses trigonometry to calculate the angle, and is more precise
 * than the "lite" implementation. This is most suitable for controllers
 * that have more resources available, and can handle floating-point
 * calculations.
 *
 * @param coordinates The coordinates read from the joystick
 * @param rotate flag to indicate if the calculated angle value should be rotated
 * @return The angle in degrees.
 */
static int16_t calculate_angle_trig(joystick_coordinate_t coordinates, bool rotate) {
    float angle = atan2(coordinates.y_coordinate, coordinates.x_coordinate) * (180.0 / M_PI);
    angle = rotate ? fmod(angle - get_stick_up_angle() + 360, 360.0) : angle;
    return fmod(angle + 360, 360.0);
}

/**
 * @brief Calculates the angle based on the joystick position.
 *
 *
 * This function calculates the angle of the joystick position
 * in degrees, ranging from 0 to 360.  This is the "lite" implementation
 * that does not use trigonometry, and is more efficient for controllers
 * that are more limited in resources, and cannot handle floating-point
 * calculations.  This implementation is less precise than the "trig"
 * implementation, but is suitable for the majority of use cases.
 *
 * @param coordinates The coordinates read from the joystick
 * @param rotate flag to indicate if the calculated angle value should be rotated
 * @return The angle in degrees.
 */
static int16_t calculate_angle_lite(joystick_coordinate_t coordinates, bool rotate) {
    // Extract x and y coordinates
    int16_t x = coordinates.x_coordinate;
    int16_t y = coordinates.y_coordinate;

    // Check for neutral position
    if (x == 0 && y == 0) return 0;

    // Determine octant using bitwise operations
    uint8_t octant = ((y < 0) << 2) | ((x < 0) << 1) | (abs(y) > abs(x));

    // Calculate ratio, swapping x and y based on octant
    uint16_t ratio = ((uint32_t)abs(octant & 1 ? x : y) << 8) / abs(octant & 1 ? y : x);

    // Calculate direction index based on octant and fine adjustment
    uint8_t dir_index = (octant << 1) | (ratio > 181 ? 2 : (ratio > 106 ? 1 : 0));

    // Look up angle from pre-calculated array
    int16_t angle = angle_to_direction[dir_index];

    // Apply rotation if needed and return final angle
    return rotate ? (angle - get_stick_up_angle() + 360) % 360 : angle;
}

/**
 * @brief Calculates the angle based on the joystick position.
 *
 * This function calculates the angle of the joystick position
 * in degrees, ranging from 0 to 360.  The calculation method
 * is determined by the profile `is_lite` flag.
 *
 * @param coordinates The coordinates read from the joystick
 * @param rotate flag to indicate if the calculated angle value should be rotated
 * @return The angle in degrees.
 */
static int16_t calculate_angle(joystick_coordinate_t coordinates, bool rotate) {
    return js_profile.is_lite ? calculate_angle_lite(coordinates, rotate) : calculate_angle_trig(coordinates, rotate);
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
static int8_t calculate_raw_direction(joystick_coordinate_t raw_coordinates) {
    // any angle with a 46 degree range around each direction is acceptable
    int8_t direction_range = 23;
    joystick_coordinate_t coordinates = check_deadzone(raw_coordinates);
    if (coordinates.x_coordinate != 0 && coordinates.y_coordinate != 0) {
        int16_t angle = calculate_angle(coordinates, false);
        for (int i = 0; i < ORIENTATION_COUNT; i++) {
            int diff = (angle - directions[i].axis_direction + 360) % 360;
            if (diff <= direction_range || diff >= (360 - direction_range)) {
                return directions[i].orientation;
            }
        }
    }
    return -1;
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
static joystick_coordinate_t read_joystick(void) {
    joystick_coordinate_t coordinates = read_joystick_coordinates();
    coordinates = check_deadzone(coordinates);
    coordinates = handle_rotation(coordinates);
    return coordinates;
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
 * @brief Handles actions for the LOWER layer based on the direction.
 *
 * This function performs specific actions when the active layer is LOWER.
 * The actions are determined by the direction provided as input.
 *
 * @param direction The direction of the joystick movement. Expected values are:
 *                 - UP: Perform the action associated with the UP direction.
 *                 - DOWN: Perform the action associated with the DOWN direction.
 *                 - RIGHT: Perform the action associated with the RIGHT direction.
 *                 - LEFT: Perform the action associated with the LEFT direction.
 */
static void handle_rgb_step_and_val(int8_t direction) {
    switch (direction) {
        case JS_UP:
            fp_rgblight_step();
            break;
        case JS_DOWN:
            fp_rgblight_step_reverse();
            break;
        case JS_RIGHT:
            fp_rgblight_increase_val();
            break;
        case JS_LEFT:
            fp_rgblight_decrease_val();
            break;
        default:
            break;
    }
}

/**
 * @brief Handles actions for the RAISE layer based on the direction.
 *
 * This function performs specific actions when the active layer is RAISE.
 * The actions are determined by the direction provided as input.
 *
 * @param direction The direction of the joystick direction. Expected values are:
 *                 - UP: Perform the action associated with the UP direction.
 *                 - DOWN: Perform the action associated with the DOWN direction.
 *                 - RIGHT: Perform the action associated with the RIGHT direction.
 *                 - LEFT: Perform the action associated with the LEFT direction.
 */
static void handle_rgb_sat_and_hue(int8_t direction) {
    switch (direction) {
        case JS_UP:
            fp_rgblight_increase_sat();
            break;
        case JS_DOWN:
            fp_rgblight_decrease_sat();
            break;
        case JS_RIGHT:
            fp_rgblight_increase_hue();
            break;
        case JS_LEFT:
            fp_rgblight_decrease_hue();
            break;
        default:
            break;
    }
}

/**
 * @brief Handles actions for the ADJUST layer based on the direction.
 *
 * This function performs specific actions when the active layer is ADJUST.
 * The actions are determined by the direction provided as input.
 *
 * @param direction The direction of the joystick direction. Expected values are:
 *                 - UP: Perform the action associated with the UP direction.
 *                 - DOWN: Perform the action associated with the DOWN direction.
 *                 - RIGHT: Perform the action associated with the RIGHT direction.
 *                 - LEFT: Perform the action associated with the LEFT direction.
 */
static void handle_rgb_toggle_and_speed(int8_t direction) {
    switch (direction) {
        case JS_UP:
            rgb_matrix_enable();
            break;
        case JS_DOWN:
            rgb_matrix_disable();
            break;
        case JS_RIGHT:
            rgb_matrix_increase_speed();
            break;
        case JS_LEFT:
            rgb_matrix_decrease_speed();
            break;
        default:
            break;
    }
}

/**
 * @brief Handles the joystick input and processes it according to the current mode.
 *
 * This function reads the joystick input, processes it based on the current mode,
 * and updates the joystick axes accordingly.
 */
static void handle_joystick(void) {
    uint8_t mods = get_mods();
    if (mods & MOD_MASK_SHIFT) {
        handle_rgb_step_and_val(calculate_direction(true));
    } else if (mods & MOD_MASK_ALT) {
        handle_rgb_sat_and_hue(calculate_direction(true));
    } else if (mods & MOD_MASK_CTRL) {
        handle_rgb_toggle_and_speed(calculate_direction(true));
    } else if (joystick_config.mode < JOYSTICK_SM_END) {
        joystick_coordinate_t coordinates = read_joystick();
        stick_modes[joystick_config.mode](coordinates.x_coordinate, coordinates.y_coordinate);
    }
}

/**
 * @brief Initializes the joystick.
 *
 * This function initializes the joystick by setting up its axes and performing
 * calibration.  It defaults the joystick to emulate arrow keys, and it sets the
 * installed "up" orientation. Note that the initialization is deferred by 100ms
 * to prevent conflicts with other keyboard initialization tasks.
 */
static uint32_t init_joystick(uint32_t triger_time, void *cb_arg) {
    setup_joystick_axes();
    calibrate_neutral_values(&joystick_calibration);
    joystick_config.mode = JS_MODE;
    joystick_config.up_orientation = JS_UP_ORIENTATION;
    return 0;
}

/**
 * @brief Initializes the joystick (deferred).
 *
 * This function initializes the joystick by setting up its axes and performing
 * calibration.  It defaults the joystick to emulate arrow keys, and it sets the
 * installed "up" orientation. Note that the initialization is deferred by 100ms
 * to prevent conflicts with other keyboard initialization tasks.
 */
void fp_post_init_joystick(void) {
    defer_exec(100, init_joystick, NULL);
}

/**
 * @brief Processes the joystick input at regular intervals.
 *
 * This function checks if the specified time interval has elapsed and, if so,
 * processes the joystick input by calling the handle_joystick function.
 */
void fp_process_joystick(void) {
    if (timer_elapsed(stick_timer) > js_profile.stick_timer_ms) {
        stick_timer = timer_read32();
        handle_joystick();
    }
}
