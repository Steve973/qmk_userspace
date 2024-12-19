#include <stdio.h>
#include "eeconfig.h"
#include "oled/oled_driver.h"
#include "quantum.h"
#include "quantum/audio/audio.h"
#include "../../../common/menu_operation.h"

uint32_t play_test_song(uint32_t trigger_time, void* cb_arg) {
    #ifdef AUDIO_ENABLE
    static float fp_startup_dark[][2] = SONG(
        HALF_DOT_NOTE(_B5),
        QUARTER_NOTE(_B5),
        HALF_NOTE(_E6),
        HALF_NOTE(_REST),
        QUARTER_NOTE(_C6),
        QUARTER_NOTE(_REST),
        QUARTER_NOTE(_G5),
        QUARTER_NOTE(_E5),
        QUARTER_NOTE(_F5),
        QUARTER_NOTE(_GS5),
        QUARTER_NOTE(_G5),
        QUARTER_NOTE(_F5),
        WHOLE_NOTE(_G5)
    );
    audio_play_melody(&fp_startup_dark, sizeof(fp_startup_dark) / sizeof(fp_startup_dark[0]), false);
    #endif
    return 0;
}

operation_result_t reset_eeprom(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        eeconfig_init();
        soft_reset_keyboard();
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t enter_bootloader(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        bootloader_jump();
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t test_piezo(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        defer_exec(100, play_test_song, NULL);
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t increase_oled_brightness(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        oled_set_brightness(MIN(oled_get_brightness() + 5, 255));
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}

operation_result_t decrease_oled_brightness(operation_result_t prev_result, void** input_values) {
    if (prev_result == OPERATION_RESULT_SUCCESS) {
        oled_set_brightness(MAX(oled_get_brightness() - 5, 10));
        return OPERATION_RESULT_SUCCESS;
    }
    return prev_result;
}
