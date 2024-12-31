#include "mfd/mfd.h"

// Load the image data for the OLED driver
#ifdef OLED_DRIVER_ENABLE
#define PINKIESOUT_LOGO_SCREEN_ENABLED
#include "images/oled/pinkiesout_logo.h"
static const image_t pinkiesout_logo = {
    .data = (const char*)&pinkiesout_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
// Load the image data for the Quantum Painter
#elif defined(QUANTUM_PAINTER_ENABLE)
#define PINKIESOUT_LOGO_SCREEN_ENABLED
#include "images/qp/pinkiesout_logo.h"
static const image_t pinkiesout_logo = {
    .data = (const uint8_t*)&pinkiesout_logo_128x128,
    .width = 128,
    .height = 128,
    .invert = false
};
#endif

// If the logo screen is enabled, define the screen elements
#ifdef PINKIESOUT_LOGO_SCREEN_ENABLED
static screen_element_t po_logo_elements[] = {
    {
        .type = CONTENT_TYPE_IMAGE,
        .x = 0,
        .y = 0,
        .content.image = &pinkiesout_logo
    }
};

// Define the screen content
const screen_content_t po_logo_screen = {
    .title = NULL,
    .elements = po_logo_elements,
    .element_count = sizeof(po_logo_elements) / sizeof(po_logo_elements[0]),
    .default_y = 0
};
#endif
