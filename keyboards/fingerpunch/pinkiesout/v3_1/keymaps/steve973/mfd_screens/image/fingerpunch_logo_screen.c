#include "mfd/mfd.h"

#ifdef OLED_DRIVER_ENABLE
#define FINGERPUNCH_LOGO_SCREEN_ENABLED
#include "images/oled/fingerpunch_logo.h"
static const image_t fingerpunch_logo = {
    .data = (const char*)&fingerpunch_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
#elif defined(QUANTUM_PAINTER_ENABLE)
#define FINGERPUNCH_LOGO_SCREEN_ENABLED
#include "images/qp/fingerpunch_logo.h"
static const image_t fingerpunch_logo = {
    .data = (const uint8_t*)&fingerpunch_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
#endif

#ifdef FINGERPUNCH_LOGO_SCREEN_ENABLED
static screen_element_t fp_logo_elements[] = {
    {
        .type = CONTENT_TYPE_IMAGE,
        .x = 0,
        .y = 0,
        .content.image = &fingerpunch_logo
    }
};

const screen_content_t fp_logo_screen = {
    .title = NULL,
    .elements = fp_logo_elements,
    .element_count = sizeof(fp_logo_elements) / sizeof(fp_logo_elements[0]),
    .default_y = 0
};
#endif
