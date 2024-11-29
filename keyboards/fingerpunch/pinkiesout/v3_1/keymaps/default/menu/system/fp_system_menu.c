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

#include "fp_system_menu.h"

const menu_item_t SYSTEM_MENU[] = {
    {"Reset EEPROM", reset_eeprom, MENU_TYPE_ACTION},
    {"Bootloader", enter_bootloader, MENU_TYPE_ACTION},
    {"Debug Info", show_debug_info, MENU_TYPE_DISPLAY},
    {"Piezo Test", test_piezo, MENU_TYPE_ACTION},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t SYSTEM_MENU_SIZE = sizeof(SYSTEM_MENU) / sizeof(menu_item_t);

char debug_info_strs[7][MAX_INFO_STR];

void init_debug_info_strings(void) {
    snprintf(debug_info_strs[0], MAX_INFO_STR, "Mfr: %s", MANUFACTURER);
    snprintf(debug_info_strs[1], MAX_INFO_STR, "KB: %s", PRODUCT);
    
    // Get version before first dash
    char version[(MAX_INFO_STR - 5)];
    strncpy(version, QMK_VERSION, sizeof(version)-1);
    version[sizeof(version)-1] = '\0';
    char* dash = strchr(version, '-');
    if (dash) *dash = '\0';
    snprintf(debug_info_strs[2], MAX_INFO_STR, "FW: %s", version);

    // Get git hash before asterisk
    char git_hash[14];
    strncpy(git_hash, QMK_GIT_HASH, sizeof(git_hash)-1);
    git_hash[sizeof(git_hash)-1] = '\0';
    char* asterisk = strchr(git_hash, '*');
    if (asterisk) *asterisk = '\0';
    snprintf(debug_info_strs[3], MAX_INFO_STR, "Git: %s", git_hash);

    snprintf(debug_info_strs[4], MAX_INFO_STR, "Matrix: %dx%d", MATRIX_ROWS, MATRIX_COLS);
    snprintf(debug_info_strs[5], MAX_INFO_STR, "RGB: %s", rgb_matrix_is_enabled() ? "On" : "Off");
    snprintf(debug_info_strs[6], MAX_INFO_STR, "Audio: %s", audio_is_on() ? "On" : "Off");
}

const menu_item_t DEBUG_INFO_DISPLAY[] = {
    {debug_info_strs[0], NULL, MENU_TYPE_INFO},
    {debug_info_strs[1], NULL, MENU_TYPE_INFO},
    {debug_info_strs[2], NULL, MENU_TYPE_INFO},
    {debug_info_strs[3], NULL, MENU_TYPE_INFO},
    {debug_info_strs[4], NULL, MENU_TYPE_INFO},
    {debug_info_strs[5], NULL, MENU_TYPE_INFO},
    {debug_info_strs[6], NULL, MENU_TYPE_INFO},
    {"Back", menu_stack_pop, MENU_TYPE_ACTION},
    {"Main Menu", menu_stack_home, MENU_TYPE_ACTION}
};

const size_t DEBUG_INFO_DISPLAY_SIZE = sizeof(DEBUG_INFO_DISPLAY) / sizeof(menu_item_t);

void show_system_menu(void) {
    menu_stack_push("System Menu", SYSTEM_MENU, SYSTEM_MENU_SIZE);
}

void reset_eeprom(void) {
    eeconfig_init();
    soft_reset_keyboard();
}

void enter_bootloader(void) {
    bootloader_jump();
}

void show_debug_info(void) {
    init_debug_info_strings();
    menu_stack_push("Debug Info", DEBUG_INFO_DISPLAY, DEBUG_INFO_DISPLAY_SIZE);
}

void test_piezo(void) {
    #ifdef AUDIO_ENABLE
    // Play ascending tones
    float test_song[][2] = {
        {NOTE_C4, 200},
        {NOTE_E4, 200},
        {NOTE_G4, 200},
        {NOTE_C5, 4}
    };
    
    PLAY_SONG(test_song);
    #endif
}
