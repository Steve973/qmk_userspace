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

#ifdef OLED_ENABLE
  #undef OLED_DISPLAY_ADDRESS
  #define OLED_DISPLAY_ADDRESS   0x3D
  #define OLED_DISPLAY_128X128
  #define OLED_PRE_CHARGE_PERIOD 0x22
  #define OLED_VCOM_DETECT       0x35
#endif

#define EECONFIG_USER_DATA_SIZE 23
#define FP_USER_CONFIG_VERSION 1
