SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_pinkiesout.c

ifeq ($(strip $(OLED_ENABLE)), yes)
	OLED_TRANSPORT = i2c
    WPM_ENABLE = yes
    KEYBOARD_SHARED_EP = yes
    SRC += menu/common/menu_core.c
	SRC += menu/common/menu_operation.c
	SRC += menu/actions/display/operation_display.c
	SRC += menu/actions/builtin/action/action.c
	SRC += menu/actions/builtin/confirmation/confirmation.c
	SRC += menu/actions/builtin/input/input.c
	SRC += menu/actions/builtin/postcondition/postcondition.c
	SRC += menu/actions/builtin/precondition/precondition.c
	SRC += menu/actions/builtin/result/result.c
	SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/oled/fp_kb_screens.c
	SRC += oled/qmk_logo.c
	SRC += oled/timeout_indicator/timeout_indicator.c
	SRC += mfd/mfd.c

    MENU_TOOL := $(QMK_USERSPACE)/users/steve973/menu/generator/generate_menu.py
    MENU_JSON := $(QMK_USERSPACE)/users/steve973/menu/config/menu_config.json

    $(INTERMEDIATE_OUTPUT)/menu_data.c: $(MENU_JSON) $(MENU_TOOL)
		python3 $(MENU_TOOL) $< $@

    SRC += $(INTERMEDIATE_OUTPUT)/menu_data.c
endif

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    QUANTUM_PAINTER_DRIVERS += sh1107_i2c
endif

ifeq ($(strip $(JOYSTICK_ENABLE)), yes)
    ANALOG_DRIVER_REQUIRED = yes
    POINTING_DEVICE_ENABLE = yes
    POINTING_DEVICE_DRIVER = custom
    SRC += joystick/fp_joystick_calibration_neutral.c
    SRC += joystick/fp_joystick_calibration_range.c
    SRC += joystick/fp_joystick.c
    SRC += joystick/fp_joystick_handler.c
    SRC += joystick/fp_joystick_read.c
endif
