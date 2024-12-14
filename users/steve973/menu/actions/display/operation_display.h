#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "menu/common/menu_core.h"
#include "menu/common/menu_operation.h"

#define MAX_DISPLAY_LINES 16
#define MAX_MESSAGE_LINES 3
#define MESSAGE_START_LINE 4
#define MAX_LINE_LENGTH 21

typedef struct {
    operation_phase_t type;
    const char* title;
    const char* messages[MAX_MESSAGE_LINES];
    union {
        struct {
            const input_config_t* inputs;
            uint8_t input_count;
        } input;
        struct {
            const char* true_text;
            const char* false_text;
        } confirm;
        struct {
            result_mode_t mode;
            const char* ok_text;
        } result;
    } phase_data;
    uint8_t selected_index;      // For navigation in current dialog
} operation_display_config_t;

// Core display function
void operation_display_message(const operation_display_config_t* config);

// Input handling
bool operation_display_process_key(uint16_t keycode, keyrecord_t *record);
int8_t operation_display_get_choice(void);
