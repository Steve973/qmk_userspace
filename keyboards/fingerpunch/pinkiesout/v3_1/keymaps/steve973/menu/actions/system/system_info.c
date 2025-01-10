#include <stdio.h>
#include <string.h>
#include "quantum/audio/audio.h"
#include "rgb_matrix/rgb_matrix.h"
#include "info_config.h"
#include "version.h"
#include "display_manager/display_manager.h"
#include "keycode_config.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/**
 * Returns device manufacturer information.
 */
static const char* get_manufacturer_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", MANUFACTURER);
    return buffer;
}

/**
 * Returns keyboard product information.
 */
static const char* get_product_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", PRODUCT);
    return buffer;
}

/**
 * Returns MCU information.
 */
static const char* get_mcu_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", TOSTRING(QMK_MCU));
    return buffer;
}

/**
 * Returns VID/PID information.
 */
static const char* get_vid_pid_info(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04X/%04X", VENDOR_ID, PRODUCT_ID);
    return buffer;
}

/**
 * Returns device version information.
 */
static const char* get_device_version(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%04X", DEVICE_VER);
    return buffer;
}

/**
 * Returns matrix size information.
 */
static const char* get_matrix_size(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%dx%d", MATRIX_ROWS, MATRIX_COLS);
    return buffer;
}

/**
 * Returns diode direction information.
 */
static const char* get_diode_direction(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", TOSTRING(DIODE_DIRECTION));
    return buffer;
}

/**
 * Returns firmware version information.
 */
static const char* get_firmware_version(void) {
    static char buffer[32];
    char version[28];
    strncpy(version, QMK_VERSION, sizeof(version) - 1);
    version[sizeof(version) - 1] = '\0';
    char* delim_pos = strchr(version, '-');
    if (delim_pos) *delim_pos = '\0';
    snprintf(buffer, sizeof(buffer), "%s", version);
    return buffer;
}

/**
 * Returns git hash information.
 */
static const char* get_git_hash(void) {
    static char buffer[32];
    char git_hash[14];
    strncpy(git_hash, QMK_GIT_HASH, sizeof(git_hash) - 1);
    git_hash[sizeof(git_hash) - 1] = '\0';
    char* delim_pos = strchr(git_hash, '*');
    if (delim_pos) *delim_pos = '\0';
    snprintf(buffer, sizeof(buffer), "%s", git_hash);
    return buffer;
}

/**
 * Returns build date information.
 */
static const char* get_build_date(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", QMK_BUILDDATE);
    return buffer;
}

/**
 * Returns RGB matrix status.
 */
static const char* get_rgb_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", rgb_matrix_is_enabled() ? "On" : "Off");
    return buffer;
}

/**
 * Returns audio status.
 */
static const char* get_audio_status(void) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s", audio_is_on() ? "On" : "Off");
    return buffer;
}

/**
 * Returns NKRO status.
 */
static const char* get_nkro_status(void) {
    static char buffer[32];
    #ifdef NKRO_ENABLE
    snprintf(buffer, sizeof(buffer), "%s", keymap_config.nkro ? "On" : "Off");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Returns mousekey status.
 */
static const char* get_mousekey_status(void) {
    static char buffer[32];
    #ifdef MOUSEKEY_ENABLE
    snprintf(buffer, sizeof(buffer), "Enabled");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Returns extrakey status.
 */
static const char* get_extrakey_status(void) {
    static char buffer[32];
    #ifdef EXTRAKEY_ENABLE
    snprintf(buffer, sizeof(buffer), "Enabled");
    #else
    snprintf(buffer, sizeof(buffer), "Disabled");
    #endif
    return buffer;
}

/**
 * Returns bootloader size information.
 */
static const char* get_bootloader_size(void) {
    static char buffer[32];
    #ifdef BOOTLOADER_SIZE
    snprintf(buffer, sizeof(buffer), "%dKB", BOOTLOADER_SIZE / 1024);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Returns firmware size information.
 */
static const char* get_firmware_size(void) {
    static char buffer[32];
    #ifdef FIRMWARE_SIZE
    snprintf(buffer, sizeof(buffer), "%dKB", FIRMWARE_SIZE / 1024);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Returns EEPROM size information.
 */
static const char* get_eeprom_size(void) {
    static char buffer[32];
    #ifdef EEPROM_SIZE
    snprintf(buffer, sizeof(buffer), "%dB", EEPROM_SIZE);
    #else
    snprintf(buffer, sizeof(buffer), "Unknown");
    #endif
    return buffer;
}

/**
 * Represents the device information screen elements.
 */
static screen_element_t device_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Manufacturer",
            .value.get_value = get_manufacturer_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Keyboard",
            .value.get_value = get_product_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "MCU",
            .value.get_value = get_mcu_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "VID/PID",
            .value.get_value = get_vid_pid_info,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Device Ver",
            .value.get_value = get_device_version,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Matrix",
            .value.get_value = get_matrix_size,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Diode Dir",
            .value.get_value = get_diode_direction,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the firmware information screen elements.
 */
static screen_element_t firmware_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Version",
            .value.get_value = get_firmware_version,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Git Hash",
            .value.get_value = get_git_hash,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Built",
            .value.get_value = get_build_date,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the feature status screen elements.
 */
static screen_element_t feature_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "RGB Matrix",
            .value.get_value = get_rgb_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Audio",
            .value.get_value = get_audio_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "NKRO",
            .value.get_value = get_nkro_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Mouse Keys",
            .value.get_value = get_mousekey_status,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Extra Keys",
            .value.get_value = get_extrakey_status,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the memory information screen elements.
 */
static screen_element_t memory_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Boot Size",
            .value.get_value = get_bootloader_size,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "FW Size",
            .value.get_value = get_firmware_size,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "EEPROM",
            .value.get_value = get_eeprom_size,
            .is_dynamic = false
        }
    }
};

/**
 * Represents the device information screen.
 */
const screen_content_t device_info_screen = {
    .title = "Device Info",
    .elements = device_info_elements,
    .element_count = sizeof(device_info_elements) / sizeof(device_info_elements[0]),
    .default_y = 2
};

/**
 * Represents the firmware information screen.
 */
const screen_content_t firmware_info_screen = {
    .title = "Firmware Info",
    .elements = firmware_info_elements,
    .element_count = sizeof(firmware_info_elements) / sizeof(firmware_info_elements[0]),
    .default_y = 2
};

/**
 * Represents the feature status screen.
 */
const screen_content_t feature_info_screen = {
    .title = "Features",
    .elements = feature_info_elements,
    .element_count = sizeof(feature_info_elements) / sizeof(feature_info_elements[0]),
    .default_y = 2
};

/**
 * Represents the memory information screen.
 */
const screen_content_t memory_info_screen = {
    .title = "Memory Info",
    .elements = memory_info_elements,
    .element_count = sizeof(memory_info_elements) / sizeof(memory_info_elements[0]),
    .default_y = 2
};
