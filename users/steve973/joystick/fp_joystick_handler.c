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
#include "joystick.h"
#include "pointing_device/pointing_device.h"
#include "quantum.h"
#include <action.h>

typedef void (*stick_mode_handler)(int8_t x, int8_t y);

static void handle_analog(int8_t x, int8_t y);
static void handle_wasd(int8_t x, int8_t y);
static void handle_arrows(int8_t x, int8_t y);
static void handle_mouse(int8_t x, int8_t y);

/**
 * @brief Array of joystick mode handlers.
 *
 * This constant array defines the different handling modes for the joystick.
 * Each mode corresponds to a specific function that processes the joystick input
 * in a different way. The available modes are:
 * - `handle_analog`: Handles analog joystick input.
 * - `handle_wasd`: Maps joystick input to WASD keys.
 * - `handle_arrows`: Maps joystick input to arrow keys.
 * - `handle_mouse`: Maps joystick input to mouse movement.
 */
static const stick_mode_handler stick_modes[] = {
    handle_analog,
    handle_wasd,
    handle_arrows,
    handle_mouse
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

    // This feels hacky, but it works, and it is the only way I know of that
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
    int8_t curr_state = (curr > 40) - (curr < -40);
    int8_t prev_state = (prev > 40) - (prev < -40);
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
 * @brief Handles mouse movement based on joystick input.
 *
 * This function scales the joystick input to the configured range and sets the mouse
 * report to the scaled values.
 *
 * @param x The x-axis value.
 * @param y The y-axis value.
 */
static void handle_mouse(int8_t x, int8_t y) {
    report_mouse_t report = pointing_device_get_report();
    report.x = x;
    report.y = -y;
    pointing_device_set_report(report);
}

/**
 * @brief Handles the joystick input and processes it according to the current mode.
 *
 * This function reads the joystick input, processes it based on the current mode,
 * and updates the joystick axes accordingly.
 */
void handle_joystick(void) {
    joystick_coordinate_t coordinates = read_joystick();
    stick_modes[joystick_config.mode](coordinates.x_coordinate, coordinates.y_coordinate);
}
