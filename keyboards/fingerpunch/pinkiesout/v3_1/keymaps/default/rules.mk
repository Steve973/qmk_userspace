ifeq ($(strip $(OLED_ENABLE)), yes)
	OLED_TRANSPORT = i2c
    WPM_ENABLE = yes
    KEYBOARD_SHARED_EP = yes
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/common/fp_menu_common.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/joystick/fp_joystick_calibration_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/joystick/fp_joystick_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/joystick/fp_joystick_mode_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/joystick/fp_joystick_sensitivity_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/main/fp_main_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_hue_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_mode_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_saturation_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_speed_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/rgb/fp_rgb_value_menu.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/menu/system/fp_system_menu.c
endif

ifeq ($(strip $(JOYSTICK_ENABLE)), yes)
    ANALOG_DRIVER_REQUIRED = yes
    POINTING_DEVICE_ENABLE = yes
    POINTING_DEVICE_DRIVER = custom
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/joystick_calibration/fp_joystick_calibration_neutral.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/joystick_calibration/fp_joystick_calibration_range.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/fp_joystick.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/fp_joystick_handler.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/default/fp_joystick_read.c
endif
