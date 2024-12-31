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

#include "config.h"
#include "debug.h"
#include "eeconfig.h"
#include "joystick.h"
#include "timer.h"
#include "fp_joystick.h"

joystick_profile_t js_profile_raw = JOYSTICK_PROFILE_RAW;
joystick_profile_t js_profile_out = JOYSTICK_PROFILE_OUT;
joystick_calibration_t joystick_calibration;
fp_joystick_config_t joystick_config;
fp_config_user_t fp_kb_config_user;

static uint32_t stick_timer = 0;

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
 * @brief Gets the current joystick mode.
 *
 * This function retrieves the current joystick mode from the joystic configuration.
 */
int8_t get_stick_mode(void) {
    return joystick_config.mode;
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
int16_t get_stick_up_angle(void) {
    return joystick_config.up_orientation * 90;
}

/**
 * @brief Initializes the user EEPROM datablock with default values.
 */
void eeconfig_init_user_datablock(void) {
    fp_kb_config_user = (fp_config_user_t) {
        .js_config = (fp_joystick_config_t) {
            .mode = JOYSTICK_SM_ARROWS,
            .up_orientation = JS_UP,
            .read_interval_ms = STICK_READ_INTERVAL_MS_DEFAULT
        },
        .js_calibration = (joystick_calibration_t) {
            .x_neutral = js_profile_out.neutral,
            .x_min = js_profile_out.min,
            .x_max = js_profile_out.max,
            .y_neutral = js_profile_out.neutral,
            .y_min = js_profile_out.min,
            .y_max = js_profile_out.max,
            .deadzone_inner_percent = DEADZONE_INNER_PERCENT_DEFAULT,
            .deadzone_outer_percent = DEADZONE_OUTER_PERCENT_DEFAULT,
            .last_neutral_cal = 0,
            .last_range_cal = 0,
            .shift_factor = js_profile_raw.bits - js_profile_out.bits
        },
        .config_version = FP_USER_CONFIG_VERSION
    };
    fp_kb_config_save();
}

/**
 * @brief Loads the user EEPROM datablock into the joystick configuration.
 *
 * This function reads the user EEPROM datablock and checks if the configuration
 * version and size match the expected values.
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool fp_kb_config_load(void) {
    eeconfig_read_user_datablock(&fp_kb_config_user);
    dprintf("Config version: %d, wanted version: %d, match: %d\n", fp_kb_config_user.config_version, FP_USER_CONFIG_VERSION, fp_kb_config_user.config_version == FP_USER_CONFIG_VERSION);
    dprintf("Config size: %d, wanted size: %d, match: %d\n", sizeof(fp_kb_config_user), EECONFIG_USER_DATA_SIZE, sizeof(fp_kb_config_user) == EECONFIG_USER_DATA_SIZE);
    return ((fp_kb_config_user.config_version == FP_USER_CONFIG_VERSION) &&
            (sizeof(fp_kb_config_user) == EECONFIG_USER_DATA_SIZE));
}

/**
 * @brief Saves the joystick configuration to the user EEPROM datablock.
 */
void fp_kb_config_save(void) {
    eeconfig_update_user_datablock(&fp_kb_config_user);
}

/**
 * @brief Initializes the joystick (deferred).
 *
 * This function initializes the joystick by setting up its axes and performing
 * calibration.  It defaults the joystick to emulate arrow keys, and it sets the
 * installed "up" orientation.
 */
void fp_post_init_joystick(void) {
    bool config_valid = fp_kb_config_load();
    if (!config_valid) {
        eeconfig_init_user_datablock();
    }
    joystick_config = fp_kb_config_user.js_config;
    joystick_calibration = fp_kb_config_user.js_calibration;
    calibrate_neutral_values(true);
    dprintf("JS mode: %d, up: %d\n",
            fp_kb_config_user.js_config.mode,
            fp_kb_config_user.js_config.up_orientation);
    dprintf("Neutral x,y: %d,%d\n",
            fp_kb_config_user.js_calibration.x_neutral,
            fp_kb_config_user.js_calibration.y_neutral);
}

/**
 * @brief Processes the joystick input at regular intervals.
 *
 * This function checks if the specified time interval has elapsed and, if so,
 * processes the joystick input by calling the handle_joystick function.
 */
void fp_process_joystick(void) {
    if (timer_elapsed(stick_timer) > joystick_config.read_interval_ms) {
        stick_timer = timer_read32();
        handle_joystick();
    }
}
