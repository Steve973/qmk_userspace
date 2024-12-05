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
#include "deferred_exec.h"
#include "quantum/action.h"
#include "quantum/quantum_keycodes.h"
#include "quantum/rgb_matrix/rgb_matrix.h"
#include "quantum/wpm.h"
#include "debug.h"
#include "print.h"
#include "fingerpunch/pinkiesout/v3_1/v3_1.h"
#include "fingerpunch/pinkiesout/v3_1/config.h"
#include "fp_joystick.h"
#include "menu/common/fp_menu_common.h"

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _QWERTY,
    _LOWER,
    _RAISE,
    _ADJUST
};

enum custom_keycodes {
    QWERTY = SAFE_RANGE,
    LOWER,
    RAISE,
    ADJUST
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_pinkiesout(
        KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,      KC_5,      KC_GRV,                       KC_BSLS,   KC_6,      KC_7,      KC_8,      KC_9,      KC_0,      KC_BSPC,
        KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,      KC_T,      KC_MINS,                      KC_EQL,    KC_Y,      KC_U,      KC_I,      KC_O,      KC_P,      KC_DEL,
        KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,      KC_G,      KC_LBRC,                      KC_RBRC,   KC_H,      KC_J,      KC_K,      KC_L,      KC_SCLN,   KC_QUOT,
        KC_LSFT,   KC_Z,      KC_X,      KC_C,      KC_V,      KC_B,      ADJUST,                       ADJUST,    KC_N,      KC_M,      KC_COMM,   KC_DOT,    KC_SLSH,   KC_RSFT,
                              KC_LCTL,   KC_LGUI,   LOWER,     KC_ENT,    KC_LALT,  XXXXXXX,  XXXXXXX,  KC_RALT,   KC_SPC,    RAISE,     KC_RGUI,   KC_RCTL
    ),

    [_LOWER] = LAYOUT_pinkiesout(
        KC_HOME,   _______,   _______,   _______,   _______,   _______,   KC_INS,                       KC_INS,    _______,   _______,   _______,   _______,   _______,   KC_HOME,
        KC_END,    _______,   _______,   KC_UP,     _______,   _______,   KC_PSCR,                      KC_PSCR,   _______,   _______,   KC_UP,     _______,   _______,   KC_END,
        KC_PGUP,   _______,   KC_LEFT,   KC_DOWN,   KC_RGHT,   _______,   KC_SCRL,                      KC_SCRL,   _______,   KC_LEFT,   KC_DOWN,   KC_RGHT,   _______,   KC_PGUP,
        KC_PGDN,   _______,   _______,   _______,   _______,   _______,   _______,                      _______,   _______,   _______,   _______,   _______,   _______,   KC_PGDN,
                              _______,   _______,   _______,   _______,   _______,  XXXXXXX,  XXXXXXX,  _______,   _______,   _______,   _______,   _______
    ),

    [_RAISE] = LAYOUT_pinkiesout(
        KC_HOME,   _______,   _______,   _______,   _______,   _______,   KC_INS,                       KC_INS,    _______,   _______,   _______,   _______,   _______,   KC_HOME,
        KC_END,    _______,   _______,   KC_UP,     _______,   _______,   KC_PSCR,                      KC_PSCR,   _______,   _______,   KC_UP,     _______,   _______,   KC_END,
        KC_PGUP,   _______,   KC_LEFT,   KC_DOWN,   KC_RGHT,   _______,   KC_SCRL,                      KC_SCRL,   _______,   KC_LEFT,   KC_DOWN,   KC_RGHT,   _______,   KC_PGUP,
        KC_PGDN,   _______,   _______,   _______,   _______,   _______,   _______,                      _______,   _______,   _______,   _______,   _______,   _______,   KC_PGDN,
                              _______,   _______,   _______,   _______,   _______,  XXXXXXX,  XXXXXXX,  _______,   _______,   _______,   _______,   _______
    ),

    [_ADJUST] = LAYOUT_pinkiesout(
        QK_BOOT,   KC_F1,     KC_F2,     KC_F3,     KC_F4,     KC_F5,     KC_F6,                        KC_F7,     KC_F8,     KC_F9,     KC_F10,    KC_F11,    KC_F12,    _______,
        RGB_TOG,   RGB_MOD,   RGB_VAI,   RGB_SAI,   RGB_HUI,   RGB_SPI,   _______,                      RGB_TOG,   RGB_MOD,   RGB_VAI,   RGB_SAI,   RGB_HUI,   RGB_SPI,   _______,
        EE_CLR,    RGB_RMOD,  RGB_VAD,   RGB_SAD,   RGB_HUD,   RGB_SPD,   _______,                      _______,   RGB_RMOD,  RGB_VAD,   RGB_SAD,   RGB_HUD,   RGB_SPD,   _______,
        _______,   _______,   _______,   _______,   _______,   _______,   _______,                      _______,   _______,   _______,   _______,   _______,   _______,   _______,
                              _______,   _______,   _______,   _______,   _______,  XXXXXXX,  XXXXXXX,  _______,   _______,   _______,   _______,   _______
    )
};

static uint32_t keypress_count = 0;
static uint8_t peak_wpm = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        keypress_count++;
    }

    if (get_in_menu_mode()) {
        return process_menu_record(keycode, record);
    }

    switch (keycode) {
        case KC_M:
            if (record->event.pressed && (get_mods() & MOD_MASK_ALT)) {
                set_in_menu_mode(true);
                return false;
            }
            break;
        case LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            }
            return false;
        case RAISE:
            if (record->event.pressed) {
                layer_on(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            }
            return false;
        case ADJUST:
            if (record->event.pressed) {
                layer_on(_ADJUST);
            } else {
                layer_off(_ADJUST);
            }
            return false;
        default:
            break;
    }

    return true;
}

void keyboard_post_init_user() {
    debug_enable = true;
    #ifdef JOYSTICK_ENABLE
    fp_post_init_joystick();
    #endif
}

void housekeeping_task_user(void) {
    #ifdef JOYSTICK_ENABLE
    fp_process_joystick();
    #endif
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    oled_set_brightness(50);
    return OLED_ROTATION_180;
}

static void render_logo(void) {
    // fp-po-v31, 128x50px
    static const char fp_logo [] PROGMEM = {
        0x00, 0x00, 0xc0, 0xf0, 0xf0, 0xf8, 0xfc, 0x7c, 0x7c, 0x3c, 0x3c, 0x7c, 0x7c, 0x7c, 0x78, 0x38, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xf8, 0xfc, 0x7c, 0x3c, 0x3c, 0x3c, 0x7c, 
        0x7c, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 
        0xf8, 0xfc, 0x00, 0x00, 0x0e, 0x7f, 0xff, 0xff, 0xff, 0xe1, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 
        0xc0, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0x60, 0x60, 0xe0, 0xc0, 
        0x00, 0xc0, 0xe0, 0x00, 0xc0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0xe0, 0xc0, 0x00, 0xe0, 0xc0, 0x00, 
        0x00, 0xc0, 0xe0, 0x00, 0xc0, 0xe0, 0x00, 0xc0, 0xe0, 0xe0, 0x60, 0x60, 0x40, 0x00, 0xc0, 0xe0, 
        0xe0, 0x60, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xef, 0xc7, 
        0x83, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x83, 0xc7, 0xef, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 
        0xff, 0xef, 0xc7, 0x83, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x7f, 0x06, 0x06, 0x07, 0x03, 
        0x00, 0x7f, 0x3f, 0x00, 0x3f, 0x7f, 0x01, 0x03, 0x07, 0x0e, 0x7f, 0x3f, 0x00, 0x7f, 0x3f, 0x06, 
        0x0f, 0x3f, 0x79, 0x00, 0x7f, 0x3f, 0x00, 0x3f, 0x7f, 0x76, 0x66, 0x60, 0x20, 0x00, 0x23, 0x67, 
        0x66, 0x76, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x9f, 
        0xff, 0xff, 0xfe, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0x9f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x01, 0x03, 
        0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xfc, 0x1c, 0x0c, 0x0c, 0x1c, 
        0xfc, 0xf8, 0x00, 0xf8, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xf8, 0x00, 0x08, 0x0c, 0x0c, 0x0c, 
        0xfc, 0xfc, 0x0c, 0x0c, 0x0c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xfc, 0x00, 
        0x00, 0x00, 0x80, 0xfc, 0xf8, 0x00, 0x08, 0x0c, 0xcc, 0xcc, 0xfc, 0x38, 0x00, 0x00, 0x00, 0x00, 
        0x30, 0x38, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xfc, 0xfe, 0xff, 0x7f, 
        0x1f, 0x0f, 0x07, 0x03, 0x03, 0x07, 0x0f, 0x1f, 0xff, 0xff, 0xfe, 0xf8, 0xf0, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0f, 0x0e, 0x0c, 0x0c, 0x0e, 
        0x0f, 0x07, 0x00, 0x07, 0x0f, 0x0e, 0x0c, 0x0c, 0x0e, 0x0f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x0f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0e, 
        0x0e, 0x07, 0x03, 0x01, 0x00, 0x00, 0x04, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, 0x00, 0x0c, 0x0c, 0x00, 
        0x08, 0x0c, 0x0f, 0x0f, 0x0c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0x3f, 0x7f, 0xfe, 
        0xf8, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf8, 0xf8, 0xff, 0x7f, 0x3f, 0x1f, 0x07, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    oled_write_raw_P(fp_logo, sizeof(fp_logo));
}

void render_layer_status(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state|default_layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("Qwerty\n"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("Lower\n"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("Raise\n"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("Adjust\n"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined\n"), false);
    }
}

void render_joystick_status(void) {
    #ifdef JOYSTICK_ENABLE
    oled_write_P(PSTR("Joystick: "), false);
    switch (get_stick_mode()) {
        case JOYSTICK_SM_ARROWS:
            oled_write_P(PSTR("Arrows\n"), false);
            break;
        case JOYSTICK_SM_WASD:
            oled_write_P(PSTR("WASD\n"), false);
            break;
        case JOYSTICK_SM_ANALOG:
            oled_write_P(PSTR("Analog\n"), false);
            break;
        case JOYSTICK_SM_MOUSE:
            oled_write_P(PSTR("Mouse\n"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined\n"), false);
    }
    #endif // JOYSTICK_ENABLE
}

void render_kb_led_status(void) {
    // Write host Keyboard LED Status to OLED
    led_t led_usb_state = host_keyboard_led_state();
    char lock_status[13] = {0};
    oled_write_P(PSTR("Lock: "), false);
    if (led_usb_state.caps_lock || led_usb_state.num_lock || led_usb_state.scroll_lock) {
        strcat(lock_status, led_usb_state.caps_lock ? PSTR("CAP ") : PSTR("    "));
        strcat(lock_status, led_usb_state.num_lock ? PSTR("NUM ") : PSTR("    "));
        strcat(lock_status, led_usb_state.scroll_lock ? PSTR("SCR") : PSTR("    "));
        oled_write(lock_status, false);
    } else {
        oled_write_P(PSTR("None           "), false);
    }
}

void render_mod_status(void) {
    // Write host Keyboard Mod Status to OLED
    uint8_t mods = get_mods();
    char mod_status[16] = {0};
    oled_write_P(PSTR("Mods: "), false);
    if (mods != 0) {
        strcat(mod_status, mods & MOD_MASK_SHIFT ? PSTR("SHF ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_CTRL ? PSTR("CTL ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_ALT ? PSTR("ALT ") : PSTR("    "));
        strcat(mod_status, mods & MOD_MASK_GUI ? PSTR("GUI") : PSTR("   "));
        oled_write(mod_status, false);
    } else {
        oled_write_P(PSTR("None           "), false);
    }
}

void render_wpm(void) {
    char wpm_str[15];
    uint8_t current = get_current_wpm();
    if (current > peak_wpm) {
        peak_wpm = current;
    }
    sprintf(wpm_str, "WPM: %3d (%3d)", current, peak_wpm);
    oled_write_P(PSTR(wpm_str), false);
}

void render_uptime(void) {
    uint32_t elapsed = timer_read32() / 1000;
    char uptime_str[17];
    sprintf(uptime_str, "Uptime: %02lu:%02lu", elapsed/60, elapsed%60);
    oled_write_P(PSTR(uptime_str), false);
}

void render_keycount(void) {
    char count_str[18];
    sprintf(count_str, "Keycount: %lu", keypress_count);
    oled_write_P(PSTR(count_str), false);
}

void render_rgb_status(void) {
    #ifdef RGB_MATRIX_ENABLE
    if (rgb_matrix_is_enabled()) {
        char rgb_str[22];
        sprintf(rgb_str, "HSV:%3d,%3d,%3d M:%d", 
            rgb_matrix_get_hue(),
            rgb_matrix_get_sat(), 
            rgb_matrix_get_val(),
            rgb_matrix_get_mode());
        oled_write_P(PSTR(rgb_str), false);
    } else {
        oled_write_P(PSTR("RGB: Off             "), false);
    }
    #endif
}

void render_key_rates(void) {
    static uint32_t last_time = 0;
    static uint32_t last_keycount = 0;
    static uint16_t keys_per_sec = 0;
    static uint16_t keys_per_min = 0;
    
    uint32_t now = timer_read32();
    uint32_t elapsed = now - last_time;
    
    // Update every second
    if (elapsed >= 1000) {
        keys_per_sec = keypress_count - last_keycount;
        keys_per_min = keys_per_sec * 60;  // Extrapolate to per minute
        last_keycount = keypress_count;
        last_time = now;
    }
    
    char rate_str[20];
    sprintf(rate_str, "KPS:%2d KPM:%4d", keys_per_sec, keys_per_min);
    oled_write_P(PSTR(rate_str), false);
}

bool oled_task_user(void) {
    static int32_t display_timer = 0;
    // Update display every half-second
    if (timer_read32() - display_timer >= 200) {
        display_timer = timer_read32();
        if (get_in_menu_mode()) {
            display_current_menu();
        } else {
            oled_set_cursor(0, 1);
            render_layer_status();
            oled_set_cursor(0, 2);
            render_joystick_status();
            oled_set_cursor(0, 3);
            render_rgb_status();
            oled_set_cursor(0, 4);
            render_key_rates();
            oled_set_cursor(0, 5);
            render_wpm();
            oled_set_cursor(0, 6);
            render_keycount();
            oled_set_cursor(0, 7);
            render_mod_status();
            oled_set_cursor(0, 8);
            render_kb_led_status();
            oled_set_cursor(0, 9);
            render_uptime();
            oled_set_cursor(0, 10);
            render_logo();
        }
    }
    return false;
}

#endif // OLED_ENABLE
