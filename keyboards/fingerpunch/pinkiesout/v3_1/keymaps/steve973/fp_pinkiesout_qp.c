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

#include "quantum/painter/qp.h"
#include "drivers/painter/sh1106/qp_sh1106.h"
#include "keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/config.h"
#include "display_manager/fonts/oled_font.qff.h"
#include "mfd/mfd.h"
#include "fp_pinkiesout.h"

painter_device_t display;
painter_font_handle_t font;

void init_display(void) {
    font = qp_load_font_mem(&oled_font);
    display = qp_sh1106_make_i2c_device(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_ADDRESS);
    qp_init(display, QP_ROTATION_180);
    #ifdef MFD_ENABLE
        mfd_init();
    #endif
}
