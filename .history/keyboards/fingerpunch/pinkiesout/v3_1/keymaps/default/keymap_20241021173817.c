/* Copyright 2021 Sadek Baroudi
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

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _QWERTY,
    _COLEMAK,
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

js_rgb_layer_t js_rgb_layers[] = {
    [0] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {0, 0, 0}
    },
    [1] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {0, 0, 255}
    },
    [2] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {0, 255, 0}
    },
    [3] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 0, 0}
    },
    [4] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 0}
    },
    [5] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 0, 255}
    },
    [6] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {0, 255, 255}
    },
    [7] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [8] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [9] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [10] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [11] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [12] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [13] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [14] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [15] = {
        .mode = JS_RGB_MODE_STATIC,
        .color = {255, 255, 255}
    },
    [16] = {
        .mode = JS_RGB_MODE_STATIC,
        .

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_QWERTY] = LAYOUT_pinkiesout(
    KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,      KC_5,      KC_GRV,                       KC_BSLS,   KC_6,      KC_7,      KC_8,      KC_9,      KC_0,      KC_BSPC,
    KC_TAB,    KC_Q,      KC_W,      KC_E,      KC_R,      KC_T,      KC_MINS,                      KC_EQL,    KC_Y,      KC_U,      KC_I,      KC_O,      KC_P,      KC_DEL,
    KC_CAPS,   KC_A,      KC_S,      KC_D,      KC_F,      KC_G,      KC_LBRC,                      KC_RBRC,   KC_H,      KC_J,      KC_K,      KC_L,      KC_SCLN,   KC_QUOT,
    KC_LSFT,   KC_Z,      KC_X,      KC_C,      KC_V,      KC_B,      ADJUST,                       ADJUST,    KC_N,      KC_M,      KC_COMM,   KC_DOT,    KC_SLSH,   KC_RSFT,
                          KC_LCTL,   KC_LGUI,   LOWER,     KC_ENT,    KC_LALT,  XXXXXXX,  XXXXXXX,  KC_RALT,   KC_SPC,    RAISE,     KC_RGUI,   KC_RCTL
),


[_COLEMAK] = LAYOUT_pinkiesout(
    KC_ESC,    KC_1,      KC_2,      KC_3,      KC_4,      KC_5,      KC_LPRN,                      KC_RPRN,   KC_6,      KC_7,      KC_8,      KC_9,      KC_0,      KC_BSPC,
    KC_TAB,    KC_Q,      KC_W,      KC_F,      KC_P,      KC_B,      KC_LBRC,                      KC_RBRC,   KC_J,      KC_L,      KC_U,      KC_Y,      KC_SCLN,   KC_BSLS,
    KC_CAPS,   KC_A,      KC_R,      KC_S,      KC_T,      KC_G,      KC_LCBR,                      KC_RCBR,   KC_M,      KC_N,      KC_E,      KC_I,      KC_O,      KC_QUOT,
    KC_LSFT,   KC_Z,      KC_X,      KC_C,      KC_D,      KC_V,      ADJUST,                       ADJUST,    KC_K,      KC_H,      KC_COMM,   KC_DOT,    KC_SLSH,   KC_RSFT,
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

[_ADJUST] =  LAYOUT_pinkiesout(
    QK_BOOT,     KC_F1,   KC_F2,     KC_F3,     KC_F4,     KC_F5,     KC_F6,                        KC_F7,     KC_F8,     KC_F9,     KC_F10,    KC_F11,    KC_F12,    _______,
    RGB_TOG,   RGB_MOD,   RGB_VAI,   RGB_SAI,   RGB_HUI,   RGB_SPI,   _______,                      RGB_TOG,   RGB_MOD,   RGB_VAI,   RGB_SAI,   RGB_HUI,   RGB_SPI,   _______,
    _______,   RGB_RMOD,  RGB_VAD,   RGB_SAD,   RGB_HUD,   RGB_SPD,   _______,                      _______,   RGB_RMOD,  RGB_VAD,   RGB_SAD,   RGB_HUD,   RGB_SPD,   _______,
    _______,   _______,   _______,   _______,   _______,  VJS_QUAD,   _______,                      _______,   _______,   _______,   _______,   _______,   _______,   _______,
                          _______,   _______,   _______,   _______,   _______,  XXXXXXX,  XXXXXXX,  _______,   _______,   _______,   _______,   _______
)
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Handle layer switching
    switch (keycode) {
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

    return true; // Process all other keycodes normally
}

void housekeeping_task_kb(void) {
    #if defined(VIKSTIK_ENABLE) || defined(VIKSTIK_LITE_ENABLE)
    process_vikstik();
    #endif
    housekeeping_task_user();
}


#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    oled_set_brightness(64);
    return OLED_ROTATION_180;
}

static void render_logo(void) {
    // pinkiesout logo, 128x64px
    static const char pinkiesout_logo [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0,
    0xc0, 0xe0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xfc,
    0x3e, 0x0e, 0x0f, 0x07, 0x07, 0x0f, 0x0e, 0x3e, 0xfc, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xfe, 0x3f, 0x07, 0x03, 0x01,
    0x01, 0x01, 0x01, 0x01, 0xc3, 0xff, 0xff, 0x3e, 0x1e, 0x0e, 0x0e, 0x0e, 0x0e, 0x1e, 0x3c, 0xfc,
    0xf8, 0xe0, 0xf0, 0xf0, 0x70, 0x70, 0x70, 0xf0, 0xe0, 0xe0, 0xc0, 0xf8, 0xff, 0x3f, 0x0f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xfe, 0x7f, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0xf0, 0xfe, 0x7f, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0x7f,
    0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf3, 0xff, 0x7f, 0x0f, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0xf0, 0xfe, 0x7f, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0x78,
    0x38, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0e, 0x06, 0x06, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x0f, 0x1e, 0xfe, 0xfc, 0xf0, 0x80, 0xf0, 0xfe, 0xff, 0x1f, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xff, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0xe0, 0xfe, 0xff, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
    0x1c, 0x18, 0x1c, 0x1c, 0x1c, 0x1f, 0x0f, 0x07, 0xff, 0xff, 0xff, 0x87, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0xc0, 0xfc, 0xff, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xff,
    0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x7f, 0xff, 0xf8, 0xc0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x0e, 0x0e, 0x0e,
    0x0f, 0x07, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0x3f, 0x07, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x1f,
    0x3e, 0x7c, 0xf0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0x7c, 0x3f, 0x1f, 0x07, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x0e, 0x1e, 0x1c, 0x38, 0x38, 0x78, 0x70, 0x70, 0x70,
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0x70, 0x70, 0x70, 0x78, 0x38, 0x38,
    0x1c, 0x1e, 0x0e, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    oled_write_raw_P(pinkiesout_logo, sizeof(pinkiesout_logo));
}

void render_status(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state|default_layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("Qwerty\n"), false);
            break;
        case _COLEMAK:
            oled_write_P(PSTR("Colemak\n"), false);
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

    // Write host Keyboard LED Status to OLEDs
    led_t led_usb_state = host_keyboard_led_state();
    oled_write_P(led_usb_state.num_lock    ? PSTR("NUMLCK ") : PSTR("       "), false);
    oled_write_P(led_usb_state.caps_lock   ? PSTR("CAPLCK ") : PSTR("       "), false);
    oled_write_P(led_usb_state.scroll_lock ? PSTR("SCRLCK ") : PSTR("       "), false);
}

bool oled_task_user(void) {
    render_status();
    // render_logo_small();
    render_logo();
    return false;
}

#endif
