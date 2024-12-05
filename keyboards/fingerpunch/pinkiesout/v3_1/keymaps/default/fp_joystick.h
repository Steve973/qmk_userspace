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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "analog.h"
#include "color.h"
#include "eeconfig.h"
#include "gpio.h"
#include "keycodes.h"
#include "quantum.h"
#include "pointing_device/pointing_device.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"
#include "config.h"

#define STICK_READ_INTERVAL_MS_DEFAULT 50
#define CALIBRATION_DURATION_MS 5000
#define CALIBRATION_SAMPLE_COUNT 100
#define FIXED_POINT_SCALE 1024
#define NEUTRAL_RECAL_DAYS 7
#define DEADZONE_INNER_PERCENT_DEFAULT 50
#define DEADZONE_OUTER_PERCENT_DEFAULT 95
#define ONE_DAY 86400

/**
 * Represents the joystick handler function signature, which is used
 * to process the joystick input for the various joystick modes.
 */
typedef void (*joystick_handler_t)(int8_t x, int8_t y);

/**
 * Represents the array of joystick handler functions, which are used
 * to process the joystick input for the various joystick modes.
 */
extern const joystick_handler_t stick_handlers[];

/**
 * Represents the number of joystick handler functions, which are used
 * to process the joystick input for the various joystick modes.
 */
extern const uint8_t stick_handler_count;

/**
 * Represents the analog joystick directions, which is useful for
 * setting the electrical orientation of the joystick, as installed,
 * that is installed in the physical "up" position.
 */
typedef enum {
    JS_NEUTRAL  = -1,
    JS_RIGHT    = 0,     // joystick's right is facing up
    JS_UP       = 1,     // joystick's up is facing up
    JS_LEFT     = 2,     // joystick's left is facing up
    JS_DOWN     = 3,     // joystick's down is facing up
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
 * Represents the joystick calibration data, including:
 * - Neutral position (center) for both axes
 * - Min/max range values for both axes
 * - Inner/outer deadzone percentages
 * - last calibration timestamps for neutral postiion and range
 * - Scale factor to map normalized values to output range
 * 
 * Processing order:
 * 1. Raw values normalized to origin (0,0)
 * 2. Deadzones applied as percentage of normalized range:
 *    - inner_deadzone_percent: Percentage (0-100) around center where no movement is detected
 *    - outer_deadzone_percent: Percentage (0-100) at extremes where max value is reported
 * 3. Scale factor applied to map normalized values to full output range
 * 
 * Scale factor ensures the normalized and deadzone-processed values 
 * reach the intended minimum/maximum output values based on the
 * joystick's physical range of motion.
 * 
 * This goes into the user EEPROM datablock, so here is some helpful
 * size information:
 * 
 * - x_neutral:              2 bytes
 * - x_min:                  2 bytes
 * - x_max:                  2 bytes
 * - y_neutral:              2 bytes
 * - y_min:                  2 bytes
 * - y_max:                  2 bytes
 * - deadzone_inner_percent: 1 byte
 * - deadzone_outer_percent: 1 byte
 * - last_neutral_cal:       2 bytes
 * - last_range_cal:         2 bytes
 * - shift_factor:           1 bytes
 * - Total size:             19 bytes
 */
typedef struct __attribute__((packed)) {
    int16_t  x_neutral, x_min, x_max;
    int16_t  y_neutral, y_min, y_max;
    uint8_t  deadzone_inner_percent, deadzone_outer_percent;
    uint16_t last_neutral_cal, last_range_cal;
    uint8_t  shift_factor;
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
 * The structure for persisted joystick configuration, including
 * the mode, and the electrical direction is installed in the physical
 * "up" direction.
 * 
 * This goes into the user EEPROM datablock, so here is some helpful
 * size information:
 * 
 * - mode:           1 byte
 * - up_orientation: 1 byte
 * - read_interval:  1 byte
 * - Total size:     3 bytes
 */
typedef struct __attribute__((packed)) {
    int8_t mode;
    int8_t up_orientation;
    uint8_t read_interval_ms;
} fp_joystick_config_t;

extern fp_joystick_config_t joystick_config;

/**
 * Represents a joystick profile, including the parameters that are
 * necessary for calibration and usage (e.g., adjustments like scaling
 * and deadzone).
 */
typedef struct {
    int8_t  bits;
    int16_t min, neutral, max;
} joystick_profile_t;

extern joystick_profile_t js_profile_raw;
extern joystick_profile_t js_profile_out;

/**
 * A macro that sets up a profile for a symmetrical 8-bit joystick.
 * This is a standard configuration that is used for joysticks that
 * have 8-bit resolution (256 steps) potentiometers for each axis,
 * and a symmetrical 8-bit output range (-127 to 127), with zero as
 * the neutral value.
 */
#define JS_PROFILE_SYM8BIT ((const joystick_profile_t) { \
    .bits = 8, \
    .min = -127, \
    .neutral = 0, \
    .max = 127 \
})

/**
 * A macro that sets up a profile for a 10-bit joystick.
 * This is a standard configuration that is used for joysticks that
 * have 10-bit resolution (1024 steps) potentiometers for each axis,
 * and an asymmetrical 10-bit output range (0 to 1023), with 512 as
 * the neutral value.
 */
#define JS_PROFILE_10BIT ((const joystick_profile_t) { \
    .bits = 10, \
    .min = 0, \
    .neutral = 512, \
    .max = 1023 \
})

#ifdef JOYSTICK_ENABLE
  #define JOYSTICK_PROFILE_RAW JS_PROFILE_10BIT
  #define JOYSTICK_PROFILE_OUT JS_PROFILE_SYM8BIT
#endif

/**
 * Represents the joystick profile, including the parameters that are
 * necessary for calibration and usage (e.g., adjustments like scaling
 * and rotation if the joystick is installed with its electrical "up"
 * direction in a physical orientation other than "up").
 */
extern joystick_profile_t js_profile;

/**
 * Represents the user configuration data, including the joystick
 * configuration and calibration data, and the configuration version.
 * 
 * This goes into the user EEPROM datablock, so here is some helpful
 * size information:
 * 
 * - js_config:      3 bytes
 * - js_calibration: 19 bytes
 * - config_version: 1 byte
 * - Total size:     23 bytes
 */
typedef struct __attribute__((packed)) {
    fp_joystick_config_t js_config;
    joystick_calibration_t js_calibration;
    uint8_t config_version;
} fp_config_user_t;

extern fp_config_user_t fp_kb_config_user;

// Joystick reading functions
int16_t read_x_axis(void);
int16_t read_y_axis(void);
joystick_coordinate_t read_joystick(void);

// Joystick coordinate processing functions
void handle_rotation(int16_t* x, int16_t* y);
void apply_deadzones(int16_t* x, int16_t* y);
void scale_joystick_coordinates(int16_t* x, int16_t* y);
void normalize_joystick_coordinates(int16_t* x, int16_t* y);
int8_t calculate_direction(bool rotate);
int16_t get_stick_up_angle(void);

// Joystick calibration functions
void calibrate_neutral_values(bool async);
void calibrate_range(bool async);

// Joystick config/properties functions
int8_t get_stick_up_orientation(void);
void set_stick_up_orientation(joystick_up_orientation up_orientation);
void set_stick_mode(uint8_t mode);
void step_stick_mode(void);
int8_t get_stick_mode(void);

// Joystick operation functions
bool wait_for_js_movement(uint32_t timeout_ms, uint8_t percent);
void handle_joystick(void);
void fp_process_joystick(void);

// Joystick initialization and eeprom functions
void fp_post_init_joystick(void);
bool fp_kb_config_load(void);
void fp_kb_config_save(void);
