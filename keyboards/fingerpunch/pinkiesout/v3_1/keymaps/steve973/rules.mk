# --- Preliminary Validation Checks ---

# Verify OLED and QUANTUM_PAINTER are not enabled at the same time
ifeq ($(and $(filter yes,$(OLED_ENABLE)),$(filter yes,$(QUANTUM_PAINTER_ENABLE))),yes)
    $(error Cannot enable both OLED_ENABLE and QUANTUM_PAINTER_ENABLE at the same time)
endif

ifeq ($(strip $(OLED_ENABLE)), yes)
    OPT_DEFS += -DOLED_ENABLE
endif
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    OPT_DEFS += -DQUANTUM_PAINTER_ENABLE
endif

# Define a helper variable for any display being enabled
DISPLAY_ENABLED := $(or $(filter yes,$(OLED_ENABLE)),$(filter yes,$(QUANTUM_PAINTER_ENABLE)))

# Validate Display Manager dependencies
ifeq ($(strip $(DISPLAY_MANAGER_ENABLE)), yes)
    OPT_DEFS += -DDISPLAY_MANAGER_ENABLE
    ifneq ($(strip $(DISPLAY_ENABLED)), yes)
        $(error DISPLAY_MANAGER_ENABLE requires either OLED_ENABLE=yes or QUANTUM_PAINTER_ENABLE=yes)
    endif
endif

# Validate MFD dependencies
ifeq ($(strip $(MFD_ENABLE)), yes)
    OPT_DEFS += -DMFD_ENABLE
    ifneq ($(strip $(DISPLAY_MANAGER_ENABLE)), yes)
        $(error MFD_ENABLE requires DISPLAY_MANAGER_ENABLE=yes)
    endif
endif

# Validate Menu dependencies
ifeq ($(strip $(MENU_ENABLE)), yes)
    OPT_DEFS += -DMENU_ENABLE
    ifneq ($(strip $(DISPLAY_MANAGER_ENABLE)), yes)
        $(error MENU_ENABLE requires DISPLAY_MANAGER_ENABLE=yes)
    endif
endif

# --- Source Files ---

# Base source files
SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_pinkiesout.c

# Display-related sources
ifeq ($(strip $(DISPLAY_ENABLED)), yes)
    WPM_ENABLE = yes
    KEYBOARD_SHARED_EP = yes

    ifeq ($(strip $(OLED_ENABLE)), yes)
        OLED_TRANSPORT = i2c
        SRC += images/oled/fp_logo_128x128.c
        SRC += timeout_indicator/timeout_indicator_oled.c
    endif

    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        QUANTUM_PAINTER_DRIVERS += sh1106_i2c
        SRC += images/qp/qmk-logo-128.qgf.c
        SRC += timeout_indicator/timeout_indicator_qp.c
    endif
endif

# Display Manager sources
ifeq ($(strip $(DISPLAY_MANAGER_ENABLE)), yes)
    SRC += display_manager/display_manager.c
    SRC += timeout_indicator/timeout_indicator.c
    ifeq ($(strip $(OLED_ENABLE)), yes)
        SRC += display_manager/display_manager_oled.c
        SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_pinkiesout_oled.c
    endif
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        SRC += display_manager/fonts/thintel15.qff.c
        SRC += display_manager/display_manager_qp.c
        SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_pinkiesout_qp.c
    endif
endif

# MFD sources
ifeq ($(strip $(MFD_ENABLE)), yes)
    OPT_DEFS += -DMFD_ENABLE
    SRC += mfd/mfd.c
    SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_kb_screens.c
endif

# Menu sources
ifeq ($(strip $(MENU_ENABLE)), yes)
    SRC += menu/common/menu_core.c
    SRC += menu/common/menu_operation.c
    SRC += menu/display/menu_display.c
    SRC += menu/actions/builtin/action/action.c
    SRC += menu/actions/builtin/confirmation/confirmation.c
    SRC += menu/actions/builtin/input/input.c
    SRC += menu/actions/builtin/postcondition/postcondition.c
    SRC += menu/actions/builtin/precondition/precondition.c
    SRC += menu/actions/builtin/result/result.c
    SRC += menu/actions/user/joystick/joystick_actions.c
    SRC += menu/actions/user/rgb/rgb_actions.c
    SRC += menu/actions/user/system/system_actions.c

    MENU_TOOL := $(QMK_USERSPACE)/users/steve973/menu/generator/generate_menu.py
    MENU_JSON := $(QMK_USERSPACE)/users/steve973/menu/config/menu_config.json

    $(INTERMEDIATE_OUTPUT)/menu_data.c: $(MENU_JSON) $(MENU_TOOL)
		python3 $(MENU_TOOL) $< $@

    SRC += $(INTERMEDIATE_OUTPUT)/menu_data.c
endif

# Joystick sources
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
