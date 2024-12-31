#include "mfd/mfd.h"

// Load the image data for the OLED driver
#ifdef OLED_DRIVER_ENABLE
#define QMK_LOGO_SCREEN_ENABLED
#include "images/oled/qmk_logo.h"
static const image_t qmk_logo = {
    .data = (const char*)&qmk_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
// Load the image data for the Quantum Painter
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

// If the logo screen is enabled, define the screen elements
#ifdef QMK_LOGO_SCREEN_ENABLED
static screen_element_t qmk_logo_elements[] = {
    {
        .type = CONTENT_TYPE_IMAGE,
        .x = 0,
        .y = 0,
        .content.image = &qmk_logo
    }
};

// Define the screen content
const screen_content_t qmk_logo_screen = {
    .title = NULL,
    .elements = qmk_logo_elements,
    .element_count = sizeof(qmk_logo_elements) / sizeof(qmk_logo_elements[0]),
    .default_y = 0
};
#endif
