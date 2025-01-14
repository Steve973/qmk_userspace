#include <stdio.h>
#include "quantum/audio/audio.h"
#include "rgb_matrix/rgb_matrix.h"
#include "keycode_config.h"
#include "display_manager/display_manager.h"

// Keep getter functions only for truly dynamic values
static const char* get_rgb_status(void) {
    return rgb_matrix_is_enabled() ? "On" : "Off";
}

static const char* get_audio_status(void) {
    return audio_is_on() ? "On" : "Off";
}

static const char* get_nkro_status(void) {
    #ifdef NKRO_ENABLE
    return keymap_config.nkro ? "On" : "Off";
    #else
    return "Disabled";
    #endif
}

static screen_element_t feature_info_elements[] = {
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "RGB Matrix",
            .value.get_value = get_rgb_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Audio",
            .value.get_value = get_audio_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "NKRO",
            .value.get_value = get_nkro_status,
            .is_dynamic = true
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Mouse Keys",
            .value.static_value =
            #ifdef MOUSEKEY_ENABLE
                "Enabled",
            #else
                "Disabled",
            #endif
            .is_dynamic = false
        }
    },
    {
        .type = CONTENT_TYPE_KEY_VALUE,
        .x = 0,
        .y = 0,
        .content.key_value = {
            .label = "Extra Keys",
            .value.static_value =
            #ifdef EXTRAKEY_ENABLE
                "Enabled",
            #else
                "Disabled",
            #endif
            .is_dynamic = false
        }
    }
};

const screen_content_t feature_info_screen = {
    .title = "Features",
    .title_highlight = HIGHLIGHT_NONE,
    .elements = feature_info_elements,
    .element_count = 5,
    .highlight_index = 0,
    .default_x = 0,
    .default_y = 2,
    .center_contents = false,
    .get_highlight_index = NULL
};
