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

ifeq ($(strip $(RGB_MATRIX_ENABLE)), yes)
	OPT_DEFS += -DRGB_MATRIX_CUSTOM_USER
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

QMK_USER_DIR := $(QMK_USERSPACE)/users/steve973
KEYMAP_DIR := keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973

# Base source files
SRC += keyboards/fingerpunch/pinkiesout/v3_1/keymaps/steve973/fp_pinkiesout.c

# Display-related sources
ifeq ($(strip $(DISPLAY_ENABLED)), yes)
    WPM_ENABLE = yes
    KEYBOARD_SHARED_EP = yes

    ifeq ($(strip $(OLED_ENABLE)), yes)
        OLED_TRANSPORT = i2c
        SRC += images/oled/fingerpunch_logo.c
        SRC += images/oled/pinkiesout_logo.c
        SRC += images/oled/qmk_logo.c
        SRC += timeout_indicator/timeout_indicator_oled.c
    endif

    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        QUANTUM_PAINTER_DRIVERS += sh1106_i2c
        SRC += images/qp/fingerpunch_logo.c
        SRC += images/qp/pinkiesout_logo.c
        SRC += images/qp/qmk_logo.c
        SRC += timeout_indicator/timeout_indicator_qp.c
    endif
endif

# Display Manager sources
ifeq ($(strip $(DISPLAY_MANAGER_ENABLE)), yes)
    SRC += display_manager/display_manager.c
    SRC += timeout_indicator/timeout_indicator.c
    ifeq ($(strip $(OLED_ENABLE)), yes)
        SRC += display_manager/display_manager_oled.c
        SRC += $(KEYMAP_DIR)/fp_pinkiesout_oled.c
    endif
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        SRC += display_manager/fonts/oled_font.qff.c
        SRC += display_manager/display_manager_qp.c
        SRC += $(KEYMAP_DIR)/fp_pinkiesout_qp.c
    endif
endif

# MFD sources
ifeq ($(strip $(MFD_ENABLE)), yes)
    OPT_DEFS += -DMFD_ENABLE
    SRC += mfd/mfd.c
    SRC += $(KEYMAP_DIR)/mfd_screens/fp_kb_screens.c
    SRC += $(KEYMAP_DIR)/mfd_screens/info/kb_status.c
    SRC += $(KEYMAP_DIR)/mfd_screens/info/key_stats.c
    SRC += $(KEYMAP_DIR)/mfd_screens/info/system_status.c
    SRC += $(KEYMAP_DIR)/mfd_screens/image/fingerpunch_logo_screen.c
    SRC += $(KEYMAP_DIR)/mfd_screens/image/pinkiesout_logo_screen.c
    SRC += $(KEYMAP_DIR)/mfd_screens/image/qmk_logo_screen.c
    ifeq ($(strip $(OLED_ENABLE)), yes)
        SRC += images/oled/fingerpunch_logo.c
        SRC += images/oled/pinkiesout_logo.c
        SRC += images/oled/qmk_logo.c
    endif
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        SRC += images/qp/fingerpunch_logo.c
        SRC += images/qp/pinkiesout_logo.c
        SRC += images/qp/qmk_logo.c
    endif
endif

# Menu sources
ifeq ($(strip $(MENU_ENABLE)), yes)
    SRC += menu/core/base/menu_core.c
    SRC += menu/core/navigation/input_handler.c
    SRC += menu/core/navigation/menu_navigation.c
    SRC += menu/core/operation/menu_operation.c
    SRC += menu/core/state/menu_state.c
    SRC += menu/core/structure/menu_item.c
    SRC += menu/display/menu_display.c
    SRC += menu/actions/builtin/action/action.c
    SRC += menu/actions/builtin/confirmation/confirmation.c
    SRC += menu/actions/builtin/input/input.c
    SRC += menu/actions/builtin/postcondition/postcondition.c
    SRC += menu/actions/builtin/precondition/precondition.c
    SRC += menu/actions/builtin/result/result.c
    SRC += menu/actions/lifecycle/operation_lifecycle_manager.c

    MENU_TOOL := $(QMK_USER_DIR)/menu/generator/generate_menu.py
    MENU_JSON_FILES ?= $(QMK_USERSPACE)/$(KEYMAP_DIR)/menu/config/menu_config.json
    MENU_DATA_FILE := $(INTERMEDIATE_OUTPUT)/menu_data.c
    MENU_GENERATED_FILES_DIR := $(QMK_USER_DIR)/menu/core/generated
    MENU_STRUCTURE_FILE := $(MENU_GENERATED_FILES_DIR)/menu_structure.txt

    MENU_JSON_FILES += $(QMK_USER_DIR)/menu/config/menu_configuration.json

    # Get all enabled features from QMK's OPT_DEFS
    MENU_DEFINES_FILE := $(INTERMEDIATE_OUTPUT)/menu_defines.txt
    $(MENU_DEFINES_FILE):
		@mkdir -p $(dir $@)
		@echo "$(foreach def,$(OPT_DEFS),$(if $(and $(findstring _ENABLE,$(def)),$(findstring -D,$(def))),$(patsubst -D%,%,$(def))))" > $@

    $(MENU_DATA_FILE): $(MENU_JSON_FILES) $(MENU_TOOL) $(MENU_DEFINES_FILE)
		python3 $(MENU_TOOL) $@ $(MENU_STRUCTURE_FILE) $(MENU_DEFINES_FILE) $(MENU_JSON_FILES)

    SRC += $(MENU_DATA_FILE)

    # User can specify their own action file locations
    MENU_ACTION_LOCATIONS ?=

    # If no locations specified, use defaults
    ifeq ($(strip $(MENU_ACTION_LOCATIONS)),)
        MENU_ACTION_LOCATIONS += $(KEYMAP_DIR)/menu/actions
        MENU_ACTION_LOCATIONS += $(QMK_USERSPACE)/$(KEYMAP_DIR)/menu/actions
        MENU_ACTION_LOCATIONS += $(QMK_USER_DIR)/actions
    endif

    # Find all .c files in specified locations (if they exist)
    MENU_ACTION_FILES := $(foreach dir,$(MENU_ACTION_LOCATIONS),$(shell find $(dir) -name '*.c' 2>/dev/null))
    # Also add the menu action files to the source list
	SRC += $(MENU_ACTION_FILES)

    # Only set up action lookup generation if we found files
    ifneq ($(strip $(MENU_ACTION_FILES)),)
        $(INTERMEDIATE_OUTPUT)/menu/core/base/menu_core.o: $(MENU_GENERATED_FILES_DIR)/menu_action_lookup.c
        MENU_ACTION_GENERATOR := $(QMK_USER_DIR)/menu/generator/generate_action_lookup.py
        $(MENU_GENERATED_FILES_DIR)/menu_action_lookup.c: $(MENU_JSON_FILES) $(MENU_ACTION_GENERATOR) $(MENU_ACTION_FILES) $(MENU_DATA_FILE) $(MENU_DEFINES_FILE)
		    python3 $(MENU_ACTION_GENERATOR) --json $(MENU_JSON_FILES) --output $@ --defines $(MENU_DEFINES_FILE) $(MENU_ACTION_FILES)

        SRC += $(MENU_GENERATED_FILES_DIR)/menu_action_lookup.c
    endif
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
