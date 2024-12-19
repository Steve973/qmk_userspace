#include <stdlib.h>
#include <stdio.h>
#include "oled_driver.h"
#include "progmem.h"
#include "operation_display.h"

static struct {
    const operation_display_config_t* current_config;
    int8_t choice_made;  // -1 if no choice made, otherwise index of choice
} display_state;

static uint8_t write_title(const char* title) {
    if (!title) return 0;
    oled_set_cursor(0, 0);
    oled_write_P(title, false);
    return 1;
}

static uint8_t write_message_block(const char* const messages[MAX_MESSAGE_LINES], uint8_t line) {
    if (!messages) return line;

    for (uint8_t i = 0; i < MAX_MESSAGE_LINES; i++) {
        if (messages[i]) {
            oled_set_cursor(0, line + i);
            oled_write_P(messages[i], false);
        }
    }
    return line + MAX_MESSAGE_LINES;
}

static uint8_t write_input_display(const input_config_t* input, uint8_t line, bool selected) {
    if (!input) return line;
    char value_text[MAX_LINE_LENGTH + 1];

    switch(input->type) {
        case INPUT_TYPE_RANGE:
            snprintf(value_text, sizeof(value_text), "%s: %s [%d-%d]",
                input->prompt, input->default_val,
                input->data.range.min, input->data.range.max);
            break;
        case INPUT_TYPE_OPTIONS:
            snprintf(value_text, sizeof(value_text), "%s: %s %s",
                input->prompt,
                input->data.options.options[atoi(input->default_val)],
                input->wrap ? "<>" : "");
            break;
        case INPUT_TYPE_CUSTOM:
            snprintf(value_text, sizeof(value_text), "%s", input->prompt);
            break;
    }

    oled_set_cursor(0, line);
    if (selected) {
        oled_write_P(PSTR(">"), false);
        oled_set_cursor(1, line);
    }
    oled_write(value_text, false);
    return line + 1;
}

static uint8_t write_confirm_display(const char* true_text, const char* false_text, uint8_t line, uint8_t selected) {
    oled_set_cursor(0, line);
    if (selected == 0) oled_write_P(PSTR(">"), false);
    oled_write_P(true_text, false);

    oled_set_cursor(0, line + 1);
    if (selected == 1) oled_write_P(PSTR(">"), false);
    oled_write_P(false_text, false);
    return line + 2;
}

void operation_display_message(const operation_display_config_t* config) {
    if (!config) return;
    uint8_t current_line = 0;

    oled_clear();
    display_state.current_config = config;
    display_state.choice_made = -1;

    // Title at top
    current_line = write_title(config->title);

    // Message block starts at fixed position
    current_line = write_message_block(config->messages, MESSAGE_START_LINE);

    // Phase-specific content starts after message block
    switch(config->type) {
        case OPERATION_PHASE_INPUT:
            if (config->phase_data.input.input_count > 1) {
                char count_text[MAX_LINE_LENGTH + 1];
                snprintf(count_text, sizeof(count_text), "Input %d of %d",
                    config->selected_index + 1,
                    config->phase_data.input.input_count);
                oled_set_cursor(0, current_line);
                oled_write(count_text, false);
                current_line++;
            }
            current_line = write_input_display(
                &config->phase_data.input.inputs[config->selected_index],
                current_line,
                true);
            break;

        case OPERATION_PHASE_CONFIRMATION:
            current_line = write_confirm_display(
                config->phase_data.confirm.true_text,
                config->phase_data.confirm.false_text,
                current_line,
                config->selected_index);
            break;

        case OPERATION_PHASE_RESULT:
            if (config->phase_data.result.mode == RESULT_MODE_ACKNOWLEDGE) {
                oled_set_cursor(0, current_line);
                oled_write_P(config->phase_data.result.ok_text, false);
                current_line++;
            }
            break;

        case OPERATION_PHASE_PRECONDITION:
        case OPERATION_PHASE_ACTION:
        case OPERATION_PHASE_POSTCONDITION:
            // These just show their message block
            break;

        default:
            break;
    }
}

bool operation_display_process_key(uint16_t keycode, keyrecord_t *record) {
    if (!display_state.current_config || !record->event.pressed) {
        return false;
    }

    const operation_display_config_t* config = display_state.current_config;

    switch (config->type) {
        case OPERATION_PHASE_INPUT:
            switch (keycode) {
                case KC_UP:
                case KC_DOWN:
                    display_state.choice_made = keycode == KC_UP ? 1 : -1;
                    return true;

                case KC_ENTER:
                    display_state.choice_made = config->selected_index;
                    return true;

                case KC_ESC:
                    display_state.choice_made = -1;
                    return true;
            }
            break;

        case OPERATION_PHASE_CONFIRMATION:
            switch (keycode) {
                case KC_UP:
                case KC_DOWN:
                    display_state.choice_made = 2;  // Signal to toggle selection
                    return true;

                case KC_ENTER:
                    display_state.choice_made = config->selected_index;
                    return true;

                case KC_ESC:
                    display_state.choice_made = -1;
                    return true;
            }
            break;

        case OPERATION_PHASE_RESULT:
            if (config->phase_data.result.mode == RESULT_MODE_ACKNOWLEDGE) {
                if (keycode == KC_ENTER || keycode == KC_ESC) {
                    display_state.choice_made = (keycode == KC_ENTER) ? 0 : -1;
                    return true;
                }
            }
            break;

        default:
            break;
    }

    return false;
}

int8_t operation_display_get_choice(void) {
    return display_state.choice_made;
}
