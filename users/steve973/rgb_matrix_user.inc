#define GET_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

RGB_MATRIX_EFFECT(raindrops_roygb)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "color.h"

/******************************************************************************
 * Rainbow ROYGB effect
 ******************************************************************************/

static const HSV roygb_colors[] = {
    {HSV_RED}, {HSV_CORAL}, {HSV_ORANGE}, {HSV_GOLDENROD}, {HSV_GOLD}, {HSV_YELLOW}, {HSV_CHARTREUSE},
    {HSV_GREEN}, {HSV_SPRINGGREEN}, {HSV_TURQUOISE}, {HSV_TEAL}, {HSV_CYAN}, {HSV_AZURE}, {HSV_BLUE}
};

static void set_raindrop_color(uint8_t i, effect_params_t* params) {
    if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) return;
    HSV hsv = {0, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v};
    uint8_t color_idx = random8_max(GET_ARRAY_SIZE(roygb_colors));
    HSV new_hsv = roygb_colors[color_idx];
    hsv.h = new_hsv.h;
    RGB rgb = rgb_matrix_hsv_to_rgb(hsv);
    rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
}

static bool raindrops_roygb(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    if (!params->init) {
        // Change one LED every tick, make sure speed is not 0
        if (scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed, 16)) % 10 == 0) {
            set_raindrop_color(random8_max(RGB_MATRIX_LED_COUNT), params);
        }
    } else {
        for (int i = led_min; i < led_max; i++) {
            set_raindrop_color(i, params);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif
