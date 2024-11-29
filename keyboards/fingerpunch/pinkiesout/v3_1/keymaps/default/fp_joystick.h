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

#pragma once

#include "gpio.h"
#include "color.h"
#include "quantum/keycodes.h"

#define CALIBRATION_DURATION_MS 5000
#define CALIBRATION_SAMPLE_COUNT 100
#define FIXED_POINT_SCALE 1024

/**
 * Represents the analog joystick directions, which is useful for
 * setting the electrical orientation of the joystick, as installed,
 * that is installed in the physical "up" position.
 */
typedef enum {
    JS_RIGHT = 0,     // joystick's right is facing up
    JS_UP    = 1,     // joystick's up is facing up
    JS_LEFT  = 2,     // joystick's left is facing up
    JS_DOWN  = 3,     // joystick's down is facing up
    ORIENTATION_COUNT = 4
} joystick_up_orientation;

/**
 * Represents the modes to designate how the joystick values are
 * interpreted.  The joystick can be used to emulate arrow keys,
 * W, A, S, and D for movement (similar to arrow keys), or it
 * can be used as-is, with analog values adjusted for the
 * configured range, or to move the mouse pointer.
 */
typedef enum {
    JOYSTICK_SM_ANALOG, // stick mode: analog
    JOYSTICK_SM_WASD,   // stick mode: wasd
    JOYSTICK_SM_ARROWS, // stick mode: arrow keys
    JOYSTICK_SM_MOUSE,  // stick mode: mouse
    JOYSTICK_SM_END     // end marker
} joystick_stick_modes;

/**
 * Represents the joystick calibration data, including the neutral
 * values for the x and y axes, min and max values for the x and y
 * axes, and the inner and outer deadzones.
 */
typedef struct joystick_calibration {
    int16_t x_neutral, x_min, x_max, y_neutral, y_min, y_max;
    int16_t deadzone_inner, deadzone_outer, scale_factor;
} joystick_calibration_t;

/**
 * Represents the joystick configuration, including the mode and
 * the electrical direction is installed in the physical "up"
 * direction.
 */
extern joystick_calibration_t joystick_calibration;

/**
 * Used to hold x and y coordinates of the joystick for easy
 * returning from a function to avoid mutation and side-effects.
 */
typedef struct {
    int16_t x_coordinate;
    int16_t y_coordinate;
} joystick_coordinate_t;

/**
 * Represents keycodes specifically for the joystick.
 */
enum joystick_keycodes {
    VJS_QUAD = QK_KB,  // stick rotation quadrants
    VJS_SMOD           // stick mode
};

/**
 * The structure for persisted joystick configuration, including
 * the mode, and the electrical direction is installed in the physical
 * "up" direction.
 */
typedef struct {
    uint8_t mode;
    int16_t up_orientation;
} fp_joystick_config_t;

extern fp_joystick_config_t joystick_config;

/**
 * The structure for blinking/pulsing the RGB of the key for a given
 * keycode. The keycode determines which key blinks/pulses, the cycle
 * duration determines the time between cycling from one color to the
 * second color, then back to the first color.  The pulse flag sets
 * the behavior between blinking or pulsing between the two colors.
 * The RGB and HSV values determine the color characteristics.
 */
typedef struct {
    uint16_t keycode, cycle_duration_ms;
    RGB      target_rgb;
    bool     pulse;
} blink_params_t;

/**
 * Represents a joystick profile, including the parameters that are
 * necessary for calibration and usage (e.g., adjustments like scaling
 * and rotation if the joystick is installed with its electrical "up"
 * direction in a physical orientation other than "up").
 */
typedef struct {
    int16_t  actuation_point, deadzone_inner, deadzone_outer, stick_timer_ms;
    int8_t   button_count, out_min, out_max, resolution_bits;
    uint16_t raw_min, raw_neutral, raw_max;
} joystick_profile_t;

/**
 * A macro that sets up a profile for an analog thumb joystick.
 * This is a standard configuration that is used for joysticks
 * that have 10-bit resolution (1024 steps) potentiometers for
 * each axis, and a symmetrical 8-bit output range (-127 to 127).
 * The actuation point is set to 40, and the deadzones are set
 * to 60, for a comfortable range to avoid accidental inputs.
 */
#define JS_10BIT_SYM8BIT ((const joystick_profile_t) { \
    .actuation_point = 40, \
    .button_count = 0, \
    .deadzone_inner = 60, \
    .deadzone_outer = 60, \
    .out_min = -127, \
    .out_max = 127, \
    .raw_min = 0, \
    .raw_neutral = 511, \
    .raw_max = 1023, \
    .resolution_bits = 8, \
    .stick_timer_ms = 10 \
})

#ifdef JOYSTICK_ENABLE
  #define JS_MODE             JOYSTICK_SM_ARROWS
  #define JS_UP_ORIENTATION   JS_RIGHT
  #define JOYSTICK_PROFILE    JS_10BIT_SYM8BIT
#endif

/**
 * Represents the joystick profile, including the parameters that are
 * necessary for calibration and usage (e.g., adjustments like scaling
 * and rotation if the joystick is installed with its electrical "up"
 * direction in a physical orientation other than "up").
 */
extern joystick_profile_t js_profile;

int8_t get_stick_up_orientation(void);
void set_stick_up_orientation(joystick_up_orientation up_orientation);
int16_t get_stick_up_angle(void);
int8_t get_stick_mode(void);
void step_stick_mode(void);
void calibrate_range(void);
void calibrate_neutral_values(void);
uint8_t get_led_index(uint16_t keycode);
void blink_key(blink_params_t blink_params, bool reset);
int16_t read_x_axis(void);
int16_t read_y_axis(void);
joystick_coordinate_t apply_scaling_and_deadzone(joystick_coordinate_t raw_coordinates);
joystick_coordinate_t read_joystick_coordinates(void);
joystick_coordinate_t read_joystick(void);
int8_t calculate_direction(bool rotate);
void joystick_adjust_oled_brightness(void);
void fp_post_init_joystick(void);
void fp_process_joystick(void);
