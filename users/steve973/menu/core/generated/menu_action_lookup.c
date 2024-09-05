#include "menu/core/operation/operation_types.h"
#include "/home/steve/projects/qmk_userspace/keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/menu/actions/joystick/joystick_actions.h"
#include "/home/steve/projects/qmk_userspace/keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/menu/actions/menu/menu_prefs_actions.h"
#include "/home/steve/projects/qmk_userspace/keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/menu/actions/rgb/rgb_actions.h"
#include "/home/steve/projects/qmk_userspace/keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/menu/actions/system/system_actions.h"

static const action_lookup_entry_t action_lookup_table[] = {
    {"calibrate_joystick_neutral", calibrate_joystick_neutral},
    {"calibrate_joystick_range", calibrate_joystick_range},
    {"detect_orientation", detect_orientation},
    {"enter_bootloader", enter_bootloader},
    {"reset_eeprom", reset_eeprom},
    {"set_joystick_mode", set_joystick_mode},
    {"set_menu_confirm_timeout", set_menu_confirm_timeout},
    {"set_menu_result_timeout", set_menu_result_timeout},
    {"set_menu_selection_style", set_menu_selection_style},
    {"set_menu_shortcut_mode", set_menu_shortcut_mode},
    {"set_menu_show_shortcuts", set_menu_show_shortcuts},
    {"set_menu_timeout_sec", set_menu_timeout_sec},
    {"set_menu_wasd_nav", set_menu_wasd_nav},
    {"set_oled_brightness", set_oled_brightness},
    {"set_rgb_hue", set_rgb_hue},
    {"set_rgb_mode", set_rgb_mode},
    {"set_rgb_sat", set_rgb_sat},
    {"set_rgb_speed", set_rgb_speed},
    {"set_rgb_val", set_rgb_val},
    {"set_x_axis_sensitivity", set_x_axis_sensitivity},
    {"set_y_axis_sensitivity", set_y_axis_sensitivity},
    {"show_debug_info", show_debug_info},
    {"show_kb_stats", show_kb_stats},
    {"test_piezo", test_piezo},
    {"toggle_rgb", toggle_rgb},
};

const size_t action_lookup_table_size = sizeof(action_lookup_table) / sizeof(action_lookup_table[0]);
const action_lookup_entry_t* const action_lookup_entries = action_lookup_table;
