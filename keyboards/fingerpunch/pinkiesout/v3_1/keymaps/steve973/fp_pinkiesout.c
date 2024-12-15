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

#include "action_layer.h"
#include "action_util.h"
#include "menu/common/menu_core.h"
#include "quantum.h"
#include "quantum/action.h"
#include "debug.h"
#include "oled/oled_driver.h"
#include "fp_pinkiesout.h"
#include "joystick/fp_joystick.h"
#include "menu/common/menu_core.h"
#include "mfd/mfd.h"

uint32_t keypress_count = 0;
uint8_t peak_wpm = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        keypress_count++;
    }

    if (is_menu_active()) {
        return process_menu_record(keycode, record);
    }

    switch (keycode) {
        case KC_M:
            if (record->event.pressed && (get_mods() & MOD_MASK_ALT)) {
                if (menu_init()) {
                    set_menu_active(true);
                } else  {
                    dprintf("Menu initialization failed\n");
                }
                return false;
            }
            break;
        case KC_LEFT:
            if (record->event.pressed && (IS_LAYER_ON(_ADJUST))) {
                increment_screen(false);
                return false;
            }
            break;
        case KC_RIGHT:
            if (record->event.pressed && (IS_LAYER_ON(_ADJUST))) {
                increment_screen(true);
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
    mfd_init();
    return OLED_ROTATION_180;
}

bool oled_task_user(void) {
    static int32_t display_timer = 0;
    // Update display every 50ms
    if (timer_read32() - display_timer >= 50) {
        display_timer = timer_read32();
        if (is_menu_active()) {
            display_current_menu();
        } else {
            render_current_screen();
        }
    }
    return false;
}

#endif // OLED_ENABLE
