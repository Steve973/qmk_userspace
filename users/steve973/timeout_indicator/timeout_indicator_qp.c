#include <stdint.h>
#include "color.h"
#include "quantum/painter/qp.h"
#include "timeout_indicator.h"

extern painter_device_t display;

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
 * The remaining height calculation follows these steps in a single expression:
 * 1. Convert elapsed time to fixed-point and cap it at the timeout duration
 * 2. Calculate progress as a ratio of elapsed to timeout time
 * 3. Invert the progress to get the remaining ratio (1 - progress)
 * 4. Scale by display height and convert back from fixed-point
 *
 * The calculation uses a fixed-point multiplier (FIXED_POINT_SCALE) to avoid
 * floating point math and maintain precision. The use of bit-shifting is fast
 * and efficient for this purpose.
 *
 * @param elapsed Time elapsed since the indicator was started.
 * @param timeout_ms Timeout duration in milliseconds.
 */
void draw_indicator(uint32_t elapsed, uint32_t timeout_ms) {
    const uint16_t height = qp_get_height(display);
    const uint16_t rightmost_col = qp_get_width(display) - 1;
    const uint16_t indicator_end = (
        height * (
            FIXED_POINT_SCALE - (
                MIN(elapsed << FIXED_POINT_BITS, FIXED_POINT_SCALE * timeout_ms)
                / timeout_ms
            )
        )
    ) >> FIXED_POINT_BITS;

    qp_line(display, rightmost_col, 0, rightmost_col, indicator_end, HSV_WHITE);
    qp_line(display, rightmost_col, indicator_end + 1, rightmost_col, height, HSV_BLACK);

    qp_flush(display);
}
