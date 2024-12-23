#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "config_common.h"
#include "rgb_matrix.h"
#include "quantum.h"
#include "version.h"
#include "info_config.h"
#include "usb_descriptor.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX_INFO_STR 32

char device_info_strs[7][MAX_INFO_STR];
char firmware_info_strs[4][MAX_INFO_STR];
char feature_info_strs[5][MAX_INFO_STR];
char memory_info_strs[3][MAX_INFO_STR];

/**
 * @brief Extracts a substring from input until the first occurrence of the
 * delimiter.
 *
 * This function copies the input string to the output buffer until the first
 * occurrence of the delimiter, or until the output buffer is full. If the
 * delimiter is found, it is replaced with a null terminator to signify the end
 * of the string.  If the output buffer is full, the string is null-terminated.
 *
 * @param output The output buffer to write to.
 * @param output_size The size of the output buffer.
 * @param input The input string to extract from.
 * @param delimiter The delimiter to stop at.
 */
static void extract_until_delimiter(char* output, size_t output_size, const char* input, char delimiter) {
    strncpy(output, input, output_size - 1);
    output[output_size - 1] = '\0';
    char* delim_pos = strchr(output, delimiter);
    if (delim_pos) *delim_pos = '\0';
}

void init_device_info_strings(void) {
    uint8_t info_str_idx = 0;
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "Mfr: %s", MANUFACTURER);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "KB: %s", PRODUCT);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "MCU: %s", MCU);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "VID/PID: %04X/%04X", VENDOR_ID, PRODUCT_ID);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "Device Ver: %04X", DEVICE_VER);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "Matrix: %dx%d", MATRIX_ROWS, MATRIX_COLS);
    snprintf(device_info_strs[info_str_idx++], MAX_INFO_STR, "Diode Dir: %s", TOSTRING(DIODE_DIRECTION));
}

// Firmware Info
void init_firmware_info_strings(void) {
    uint8_t info_str_idx = 0;

    char version[MAX_INFO_STR - 5];
    extract_until_delimiter(version, sizeof(version), QMK_VERSION, '-');

    char git_hash[14];
    extract_until_delimiter(git_hash, sizeof(git_hash), QMK_GIT_HASH, '*');

    snprintf(firmware_info_strs[info_str_idx++], MAX_INFO_STR, "FW: %s", version);
    snprintf(firmware_info_strs[info_str_idx++], MAX_INFO_STR, "Git: %s", git_hash);
    snprintf(firmware_info_strs[info_str_idx++], MAX_INFO_STR, "Built: %s", QMK_BUILDDATE);
}

// Feature Status
void init_feature_info_strings(void) {
    uint8_t info_str_idx = 0;

    char nkro_str[9] = "Disabled";
    #ifdef NKRO_ENABLE
    snprintf(nkro_str, sizeof(nkro_str), "%s", keymap_config.nkro ? "On" : "Off");
    #endif

    char mousekey_str[9] = "Disabled";
    #ifdef MOUSEKEY_ENABLE
    strcpy(mousekey_str, "Enabled");
    #endif

    char extrakey_str[9] = "Disabled";
    #ifdef EXTRAKEY_ENABLE
    strcpy(extrakey_str, "Enabled");
    #endif

    snprintf(feature_info_strs[info_str_idx++], MAX_INFO_STR, "RGB: %s", rgb_matrix_is_enabled() ? "On" : "Off");
    snprintf(feature_info_strs[info_str_idx++], MAX_INFO_STR, "Audio: %s", audio_is_on() ? "On" : "Off");
    snprintf(feature_info_strs[info_str_idx++], MAX_INFO_STR, "NKRO: %s", nkro_str);
    snprintf(feature_info_strs[info_str_idx++], MAX_INFO_STR, "MouseKey: %s", mousekey_str);
    snprintf(feature_info_strs[info_str_idx++], MAX_INFO_STR, "ExtraKey: %s", extrakey_str);
}

// Memory Info
void init_memory_info_strings(void) {
    uint8_t info_str_idx = 0;

    char bootloader_str[9] = "Unknown";
    #ifdef BOOTLOADER_SIZE
    snprintf(bootloader_str, sizeof(bootloader_str), "%dKB", BOOTLOADER_SIZE / 1024);
    #endif

    char firmware_str[9] = "Unknown";
    #ifdef FIRMWARE_SIZE
    snprintf(firmware_str, sizeof(firmware_str), "%dKB", FIRMWARE_SIZE / 1024);
    #endif

    char eeprom_str[9] = "Unknown";
    #ifdef EEPROM_SIZE
    snprintf(eeprom_str, sizeof(eeprom_str), "%dB", EEPROM_SIZE);
    #endif

    snprintf(memory_info_strs[info_str_idx++], MAX_INFO_STR, "Boot Size: %s", bootloader_str);
    snprintf(memory_info_strs[info_str_idx++], MAX_INFO_STR, "FW Size: %s", firmware_str);
    snprintf(memory_info_strs[info_str_idx++], MAX_INFO_STR, "EEPROM: %s", eeprom_str);
}
