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

#include <stdint.h>
#include <stddef.h>
#include "action_layer.h"
#include "action_util.h"
#include "keyboard.h"
#include "menu/common/menu_core.h"
#include "quantum.h"
#include "quantum/action.h"
#include "debug.h"
#include "fp_pinkiesout.h"
#include "joystick/fp_joystick.h"
#include "menu/common/menu_core.h"
#include "mfd/mfd.h"

uint32_t keypress_count = 0;
uint8_t peak_wpm = 0;

/**
 * Handles Pinkies Out key record processing.  If the menu is active, the
 * record is passed to the menu system.  Otherwise, the record is processed
 * here, if any of the relevant keys or key combinations are pressed.
 *
 * The following key combinations are handled:
 * - Alt + M:      Activates the menu system
 * - Alt + Up:     Changes to the next MFD screen collection
 * - Alt + Down:   Changes to the previous MFD screen collection
 * - Alt + Left:   Decrements the current MFD screen to the previous screen
 * - Alt + Right:  Increments the current MFD screen to the next screen
 * - Lower:        Activates the Lower layer
 * - Raise:        Activates the Raise layer
 * - Adjust:       Activates the Adjust layer
 *
 * Any other key will be processed by the keyboard firmware.
 *
 * Note that all keypresses are tallied through this function.
 *
 * @param keycode The keycode that was pressed
 * @param record The key record
 * @return false if the key was handled and processing should stop, false otherwise
 */
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
                set_menu_active(true);
                return false;
            }
            break;
        case KC_UP:
            if (record->event.pressed && (IS_LAYER_ON(_ADJUST))) {
                change_collection(true);
                return false;
            }
            break;
        case KC_DOWN:
            if (record->event.pressed && (IS_LAYER_ON(_ADJUST))) {
                change_collection(false);
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

/**
 * Handles Pinkies Out keyboard user initialization.  This function is used to
 * initialize the display and joystick, if enabled.
 */
void keyboard_post_init_user() {
    debug_enable = true;
    #ifdef QUANTUM_PAINTER_ENABLE
        init_display();
    #endif
    #ifdef JOYSTICK_ENABLE
        fp_post_init_joystick();
    #endif
}

/**
 * Handles Pinkies Out keyboard user housekeeping tasks.  This function is used
 * to process joystick input, if enabled, and to update the display, if the
 * quantum painter is enabled.  In this case, he display is updated every 50ms.
 */
void housekeeping_task_user(void) {
    #ifdef JOYSTICK_ENABLE
        fp_process_joystick();
    #endif
    #ifdef QUANTUM_PAINTER_ENABLE
        // Update display every 50ms
        static int32_t display_timer = 0;
        int32_t now = timer_read32();
        if (now - display_timer >= 50) {
            display_timer = now;
            show_current_screen();
        }
    #endif
}
