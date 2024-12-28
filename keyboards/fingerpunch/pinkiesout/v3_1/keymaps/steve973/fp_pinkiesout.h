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

#include <quantum_keycodes.h>

/**
 * Tracks the number of keypresses that have occurred.
 */
extern uint32_t keypress_count;

/**
 * Tracks the peak words per minute (WPM) that have been achieved.
 */
extern uint8_t peak_wpm;

/**
 * Represents keyboard layer names.
 */
enum layer_names {
    _QWERTY,
    _LOWER,
    _RAISE,
    _ADJUST
};

/**
 * Represents custom keycodes.
 */
enum custom_keycodes {
    QWERTY = SAFE_RANGE,
    LOWER,
    RAISE,
    ADJUST
};

#ifdef QUANTUM_PAINTER_ENABLE
void init_display(void);
#endif
