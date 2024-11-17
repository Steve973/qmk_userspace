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

#include "color.h"
#include "gpio.h"

#define CALIBRATION_DURATION_MS 5000
#define CALIBRATION_SAMPLE_COUNT 100
#define ANGLE_DIVISIONS 24

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef struct {
    int8_t  ordinal;
    int16_t angle;
} axis_values;

/**
 * Represents the axis directions in a cartesian coordinate system,
 * and their corresponding angle values.
 */
typedef enum {
    AXIS_RIGHT = 0,    // Positive x-axis
    AXIS_UP    = 90,   // Positive y-axis
    AXIS_LEFT  = 180,  // Negative x-axis
    AXIS_DOWN  = 270   // Negative y-axis
} axis_directions;

/**
 * Represents the directions of the joystick, which are used to
 * determine the angle of the joystick. This is a lookup table
 * that is better for resource-constrained processors at the
 * expense of resolution.
 */
typedef enum {
    // East quadrant I
    DIR_E = AXIS_RIGHT,  // East (0°)
    DIR_E15 = 15,        // East + 15°
    DIR_E30 = 30,        // East + 30°
    DIR_E45 = 45,        // East + 45°
    DIR_E60 = 60,        // East + 60°
    DIR_E75 = 75,        // East + 75°
    // North quadrant II
    DIR_N = AXIS_UP,     // North (90°)
    DIR_N15 = 105,       // North + 15°
    DIR_N30 = 120,       // North + 30°
    DIR_N45 = 135,       // North + 45°
    DIR_N60 = 150,       // North + 60°
    DIR_N75 = 165,       // North + 75°
    // West quadrant III
    DIR_W = AXIS_LEFT,   // West (180°)
    DIR_W15 = 195,       // West + 15°
    DIR_W30 = 210,       // West + 30°
    DIR_W45 = 225,       // West + 45°
    DIR_W60 = 240,       // West + 60°
    DIR_W75 = 255,       // West + 75°
    // South quadrant IV
    DIR_S = AXIS_DOWN,   // South (270°)
    DIR_S15 = 285,       // South + 15°
    DIR_S30 = 300,       // South + 30°
    DIR_S45 = 315,       // South + 45°
    DIR_S60 = 330,       // South + 60°
    DIR_S75 = 345        // South + 75°
} stick_direction_t;

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
 * configured range.
 */
typedef enum {
    JOYSTICK_SM_ANALOG, // stick mode: analog
    JOYSTICK_SM_WASD,   // stick mode: wasd
    JOYSTICK_SM_ARROWS, // stick mode: arrow keys
    JOYSTICK_SM_END     // end marker
} joystick_stick_modes;

/**
 * Represents the joystick calibration data, including the neutral
 * values for the x and y axes, min and max values for the x and y
 * axes, and the inner and outer deadzones.
 */
typedef struct joystick_calibration {
    int16_t x_neutral, x_min, x_max, y_neutral, y_min, y_max;
    int16_t deadzone_inner, deadzone_outer;
} joystick_calibration_t;


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
    bool     is_lite;
    pin_t    axis_pins[6];
} joystick_profile_t;

/**
 * A macro that sets up a profile for an analog thumb joystick.
 * This is a standard configuration that is used for joysticks
 * that have 10-bit resolution (1024 steps) potentiometers for
 * each axis, and a symmetrical 8-bit output range (-127 to 127).
 * The actuation point is set to 40, and the deadzones are set
 * to 60, for a comfortable range to avoid accidental inputs.
 */
#define JS_10BIT_SYM8BIT_BASE(is_lite_value) ((const joystick_profile_t) { \
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
    .stick_timer_ms = 5, \
    .is_lite = is_lite_value, \
    .axis_pins = { \
        VIK_GPIO_1, /* X axis */ \
        VIK_GPIO_2, /* Y axis */ \
        NO_PIN,     /* Z axis */ \
        NO_PIN,     /* Rx axis */ \
        NO_PIN,     /* Ry axis */ \
        NO_PIN      /* Rz axis */ \
    } \
})

// Non-lite version
#define JS_10BIT_SYM8BIT JS_10BIT_SYM8BIT_BASE(false)

// Lite version (for boards with limited math capabilities)
#define JS_10BIT_SYM8BIT_LITE JS_10BIT_SYM8BIT_BASE(true)

int8_t get_stick_up_orientation(void);
void   set_stick_up_orientation(joystick_up_orientation up_orientation);
int8_t get_stick_mode(void);
void   step_stick_mode(void);
int8_t get_stick_direction(bool rotate);
void   calibrate_range(void);
void   fp_post_init_joystick(void);
void   fp_process_joystick(void);
