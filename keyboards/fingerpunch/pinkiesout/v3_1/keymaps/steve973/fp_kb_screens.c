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

#include "display_manager/display_manager.h"
#include "mfd/mfd.h"

extern const screen_content_t kb_status_screen;
extern const screen_content_t key_stats_screen;
extern const screen_content_t system_status_screen;
extern const screen_content_t fp_logo_screen;
extern const screen_content_t po_logo_screen;
extern const screen_content_t qmk_logo_screen;

// Collection setup
static const screen_content_t* info_screens[] = {
    &kb_status_screen,
    &key_stats_screen,
    &system_status_screen
};

static const screen_content_t* image_screens[] = {
    &fp_logo_screen,
    &po_logo_screen,
    &qmk_logo_screen
};

static mfd_config_t info_screen_config = {
    .screens = (screen_content_t**)info_screens,
    .screen_count = sizeof(info_screens) / sizeof(info_screens[0]),
    .default_index = 0,
    .current_index = 0,
    .timeout_ms = 30000,
    .cycle_screens = true
};

static mfd_config_t image_screen_config = {
    .screens = (screen_content_t**)image_screens,
    .screen_count = sizeof(image_screens) / sizeof(image_screens[0]),
    .default_index = 0,
    .current_index = 0,
    .timeout_ms = 30000,
    .cycle_screens = true
};

static mfd_config_t* collection_array[] = {
    &info_screen_config,
    &image_screen_config
};

mfd_state_t mfd_state = {
    .collections = collection_array,
    .collection_count = 2,
    .active_collection = 0
};
