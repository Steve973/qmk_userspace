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
#include <stdbool.h>
#include <stdint.h>
#include "fp_joystick.h"
#include "eeprom.h"
#include "gpio.h"
#include "keycodes.h"
#include "quantum.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"

joystick_profile_t js_profile = JOYSTICK_PROFILE;

typedef void (*stick_mode_handler)(int8_t x, int8_t y);

static uint32_t stick_timer = 0;

static void handle_analog(int8_t x, int8_t y);
static void handle_wasd(int8_t x, int8_t y);
static void handle_arrows(int8_t x, int8_t y);

joystick_calibration_t joystick_calibration;
fp_joystick_config_t joystick_config;

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
 * @brief Simulates a key pressed / released event identical to a physically operated key.
 *
 * Simulates a physically pressed or released key event by executing the corresponding
 * action. This function is used to simulate key events based on joystick input.
 */
static void simulate_key_event(uint16_t keycode, bool pressed) {
    keyrecord_t event = {
        .event.pressed = pressed,
        .event.time = timer_read(),
        .event.key = (keypos_t){0, 0},
        .event.type = KEY_EVENT,
        .tap.count = 1
    };

    // This feels like "ridin' dirty" but it works, and it is the only way I know of that
    // will let the regular keypress path process this "virtual keypress" event.
    if (!process_record_user(keycode, &event)) {
        return;  // Menu handled it
    }

    action_t action;
    action.code = ACTION(ACT_MODS, keycode);
    process_action(&event, action);
}

/**
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
        simulate_key_event(key_to_handle, should_register);
        dprintf("key %d %s\n", key_to_handle, should_register ? "pressed" : "released");
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
 * @brief Handles the joystick input and processes it according to the current mode.
 *
 * This function reads the joystick input, processes it based on the current mode,
 * and updates the joystick axes accordingly.
 */
static void handle_joystick(void) {
    joystick_coordinate_t coordinates = read_joystick();
    stick_modes[joystick_config.mode](coordinates.x_coordinate, coordinates.y_coordinate);
}

/**
 * @brief Initializes the joystick (deferred).
 *
 * This function initializes the joystick by setting up its axes and performing
 * calibration.  It defaults the joystick to emulate arrow keys, and it sets the
 * installed "up" orientation.
 */
void fp_post_init_joystick(void) {
    calibrate_neutral_values();
    joystick_config.mode = JS_MODE;
    joystick_config.up_orientation = JS_UP_ORIENTATION;
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
