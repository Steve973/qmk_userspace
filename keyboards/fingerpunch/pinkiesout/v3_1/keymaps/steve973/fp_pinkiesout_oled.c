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

#include "timer.h"
#include "oled/oled_driver.h"
#include "display_manager/display_manager.h"
#include "mfd/mfd.h"
#include "fp_pinkiesout.h"

/**
 * @brief OLED Display initialization for user customization.
 *
 * This function sets the OLED brightness, and initializes the MFD if enabled.
 *
 * @param rotation The desired rotation of the display.
 */
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    oled_set_brightness(50);
    #ifdef MFD_ENABLE
        mfd_init();
    #endif
    return OLED_ROTATION_180;
}

/**
 * @brief OLED Display task for user customization.
 *
 * This function limits display updates to every 50ms, and invokes the display
 * manager's show_current_screen function to update the displayed screen
 * content.
 */
bool oled_task_user(void) {
    static int32_t display_timer = 0;
    int32_t now = timer_read32();
    // Update display every 50ms
    if (now - display_timer >= 50) {
        display_timer = now;
        show_current_screen();
    }
    return false;
}
