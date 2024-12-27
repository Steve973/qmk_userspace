#include <stdint.h>
#include "oled_driver.h"

#define FIXED_POINT_BITS 8

// 256 = 2^8 for efficient bit-shifting operations
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)

/**
 * @brief Draw a timeout indicator on the OLED display.
 *
 * This function draws a vertical progress bar on the right side of the display
 * to indicate the time remaining before a timeout occurs. The display starts
 * with a full bar extending from the top to the bottom of the display at the
 * rightmost column. As time elapses, the bar decays from the bottom to the top,
 * indicating the remaining time. When the bar reaches the top, the line of
 * pixels has fully decayed, and the timeout has occurred.
 *
 * The calculation uses a fixed-point multiplier (FIXED_POINT_SCALE) to avoid
 * floating point math and maintain precision. The progress is calculated as a
 * ratio of the elapsed time to the timeout duration, and then scaled to the
 * height of the display. The use of bit-shifting is fast and efficient for
 * this purpose.
 *
 * @param elapsed Time elapsed since the indicator was started.
 * @param timeout_ms Timeout duration in milliseconds.
 */
void draw_indicator(uint32_t elapsed, uint32_t timeout_ms) {
    uint8_t height = OLED_DISPLAY_HEIGHT;
    uint32_t progress = (elapsed << FIXED_POINT_BITS) / timeout_ms;
    if (progress > FIXED_POINT_SCALE) progress = FIXED_POINT_SCALE;
    uint16_t remaining_fixed = FIXED_POINT_SCALE - progress;
    uint8_t remaining = (height * remaining_fixed) >> 8;

    for (uint8_t i = 0; i < height; i++) {
        oled_write_pixel(OLED_DISPLAY_WIDTH-1, i, i < remaining);
    }
}
