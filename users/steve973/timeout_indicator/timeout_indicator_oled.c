#include <stdint.h>
#include "oled_driver.h"

void draw_indicator(uint32_t elapsed, uint32_t timeout_ms) {
    uint8_t height = OLED_DISPLAY_HEIGHT;
    uint16_t progress = (elapsed << 8) / timeout_ms;
    uint16_t remaining_fixed = 256 - progress;
    uint8_t remaining = (height * remaining_fixed) >> 8;

    for (uint8_t i = 0; i < height; i++) {
        oled_write_pixel(OLED_DISPLAY_WIDTH-1, i, i < remaining);
    }
}
