// Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include "rgb_matrix.h"

//----------------------------------------------------------
// RGB Matrix Effect Enum Generation
//----------------------------------------------------------
#undef RGB_MATRIX_EFFECT
#define RGB_MATRIX_EFFECT(x) RGB_MATRIX_EFFECT_##x,

enum {
    RGB_MATRIX_EFFECT_NONE,
    #include "rgb_matrix_effects.inc"
    #ifdef RGB_MATRIX_CUSTOM_KB
        #include "rgb_matrix_kb.inc"
    #endif
    #ifdef RGB_MATRIX_CUSTOM_USER
        #include "rgb_matrix_user.inc"
    #endif
    #undef RGB_MATRIX_EFFECT
};

//----------------------------------------------------------
// Effect Name Conversion
//----------------------------------------------------------
#define RGB_MATRIX_EFFECT(x)    \
    case RGB_MATRIX_EFFECT_##x: \
        return #x;

const char* rgb_matrix_name(uint8_t effect) {
    static char unknown_buf[11];  // "Effect XXX" + null
    switch (effect) {
        case RGB_MATRIX_EFFECT_NONE:
            return "NONE";
        #include "rgb_matrix_effects.inc"
        #ifdef RGB_MATRIX_CUSTOM_KB
            #include "rgb_matrix_kb.inc"
        #endif
        #ifdef RGB_MATRIX_CUSTOM_USER
            #include "rgb_matrix_user.inc"
        #endif
        #undef RGB_MATRIX_EFFECT
        default:
            snprintf(unknown_buf, sizeof(unknown_buf), "Effect %-3d", effect);
            return unknown_buf;
    }
}

//----------------------------------------------------------
// Public Interface
//----------------------------------------------------------
const char* rgb_matrix_get_effect_name(void) {
    static char buf[32] = {0};
    static uint8_t last_effect = 0;
    uint8_t current_effect = rgb_matrix_get_mode();

    if (last_effect != current_effect) {
        last_effect = current_effect;
        snprintf(buf, sizeof(buf), "%s", rgb_matrix_name(current_effect));

        // Format the string (convert MY_EFFECT to My Effect)
        for (uint8_t i = 1; i < sizeof(buf); ++i) {
            if (buf[i] == 0) {
                break;
            }
            else if (buf[i] == '_') {
                buf[i] = ' ';
            }
            else if (buf[i - 1] == ' ') {
                buf[i] = toupper(buf[i]);
            }
            else if (buf[i - 1] != ' ') {
                buf[i] = tolower(buf[i]);
            }
        }
    }

    return buf;
}
