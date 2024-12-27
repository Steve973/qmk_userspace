#include <stdint.h>
#include "quantum/painter/qp.h"

extern painter_device_t display;

void draw_indicator(uint32_t elapsed, uint32_t timeout_ms) {
    uint16_t progress = (elapsed << 8) / timeout_ms;
    uint16_t remaining_fixed = 256 - progress;
    uint16_t remaining = (qp_get_height(display) * remaining_fixed) >> 8;

    for (uint16_t i = 0; i < qp_get_height(display); i++) {
        qp_setpixel(
            display,
            qp_get_width(display) - 1,
            i,
            0,
            0,
            i < remaining ? 128 : 0);
    }

    qp_flush(display);
}
