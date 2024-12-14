#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * Multi-Function Display (MFD) for QMK OLED displays
 *
 * Default implementation shows QMK logo (128x128px).
 * To use your own logo:
 *
 * 1. Create your own oled_display_logo implementation:
 *
 * void oled_display_logo(void) {
 *     oled_set_cursor(0, 0);  // Or your preferred position
 *     // Your logo rendering code here
 * }
 *
 * 2. Place this function in your keymap.c or a separate file
 *    The weak default will be overridden automatically
 *
 * Note: If using a different size display, you MUST override
 * the default implementation, since the default QMK logo is
 * 128x128px.
 */
__attribute__((weak)) void oled_display_logo(void);

#define LOGO_SCREEN_INDEX (-1)

typedef enum {
    MFD_TYPE_SIMPLE,    // Framework-managed text/value display
    MFD_TYPE_CUSTOM     // Custom rendering function
} mfd_screen_type_t;

typedef enum {
    MFD_POSITION_AUTO,      // Framework handles positioning
    MFD_POSITION_MANUAL     // User specifies exact positions
} mfd_position_mode_t;

typedef enum {
    MFD_AUTO_ALIGN_TOP,     // Start from top (with spacing after title)
    MFD_AUTO_ALIGN_CENTER   // Vertically center the content
} mfd_auto_align_t;

// For simple text/value pairs
typedef struct {
    const char* label;              // Label for the value (e.g., "WPM")
    const char* (*get_value)(void); // Function to get current value
} mfd_value_pair_t;

typedef struct {
    const char* title;                  // Screen title
    mfd_screen_type_t type;             // Screen type
    uint16_t refresh_interval;          // In milliseconds, 0 = static
    mfd_position_mode_t position_mode;  // Auto or manual positioning
    mfd_auto_align_t auto_align;        // For auto positioning: top or center

    union {
        // For simple text/value displays
        struct {
            mfd_value_pair_t* pairs;     // Array of label/value pairs
            uint8_t pair_count;          // Number of pairs
            // Only used if position_mode is MANUAL
            uint8_t start_row;           // Starting row for manual positioning
            uint8_t start_col;           // Starting column for manual positioning
        } simple;

        // For custom rendering
        struct {
            void (*render)(void);        // Custom render function
        } custom;
    } display;
} mfd_screen_t;

typedef struct {
    mfd_screen_t* screens;         // Array of screen definitions
    uint8_t screen_count;          // Number of screens
    int8_t current_index;         // Currently displayed screen
    int8_t default_index;         // Index of the default screen
    uint32_t timeout_start;        // When current non-default screen started
    const uint32_t timeout_ms;     // 30 seconds in ms
    bool cycle_screens;            // Whether to cycle through screens, using timeout_ms
} mfd_config_t;

extern mfd_config_t mfd_config;    // Screens defined elsewhere

void mfd_init(void);
void render_current_screen(void);
