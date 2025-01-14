#include <stdio.h>
#include "info_config.h"
#include "display_manager/display_manager.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

static screen_element_t device_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Manufacturer",
            .value.static_value = MANUFACTURER,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Keyboard",
            .value.static_value = PRODUCT,
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "MCU",
            .value.static_value = TOSTRING(QMK_MCU),
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "VID/PID",
            .value.static_value = TOSTRING(VENDOR_ID) "/" TOSTRING(PRODUCT_ID),
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Device Ver",
            .value.static_value = TOSTRING(DEVICE_VER),
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Matrix",
            .value.static_value = TOSTRING(MATRIX_ROWS) "x" TOSTRING(MATRIX_COLS),
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Diode Dir",
            .value.static_value = TOSTRING(DIODE_DIRECTION),
            .is_dynamic = false
        }
    }
};

const screen_content_t device_info_screen = {
    .title = "Device Info",
    .title_highlight = HIGHLIGHT_NONE,
    .elements = device_info_elements,
    .element_count = 7,
    .highlight_index = 0,
    .default_x = 0,
    .default_y = 2,
    .center_contents = false,
    .get_highlight_index = NULL
};
