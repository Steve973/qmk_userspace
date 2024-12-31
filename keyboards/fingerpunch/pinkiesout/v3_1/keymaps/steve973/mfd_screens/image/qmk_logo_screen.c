#include "mfd/mfd.h"

#ifdef OLED_DRIVER_ENABLE
#define QMK_LOGO_SCREEN_ENABLED
#include "images/oled/qmk_logo.h"
static const image_t qmk_logo = {
    .data = (const char*)&qmk_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
#elif defined(QUANTUM_PAINTER_ENABLE)
#define QMK_LOGO_SCREEN_ENABLED
#include "images/qp/qmk_logo.h"
static const image_t qmk_logo = {
    .data = (const uint8_t*)&qmk_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
#endif

#ifdef QMK_LOGO_SCREEN_ENABLED
static screen_element_t qmk_logo_elements[] = {
    {
        .type = CONTENT_TYPE_IMAGE,
        .x = 0,
        .y = 0,
        .content.image = &qmk_logo
    }
};

const screen_content_t qmk_logo_screen = {
    .title = NULL,
    .elements = qmk_logo_elements,
    .element_count = sizeof(qmk_logo_elements) / sizeof(qmk_logo_elements[0]),
    .default_y = 0
};
#endif
