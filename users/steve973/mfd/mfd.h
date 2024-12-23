#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "display_manager/display_manager.h"

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

typedef struct {
    screen_content_t* screens;     // Array of screen definitions
    uint8_t screen_count;          // Number of screens
    int8_t current_index;          // Currently displayed screen
    int8_t default_index;          // Index of the default screen
    uint32_t timeout_start;        // When current non-default screen started
    const uint32_t timeout_ms;     // 30 seconds in ms
    bool cycle_screens;            // Whether to cycle through screens, using timeout_ms
} mfd_config_t;

extern mfd_config_t mfd_config;    // Screens defined elsewhere

void mfd_init(void);
void increment_screen(bool positive_increment);
