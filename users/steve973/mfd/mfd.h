#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "display_manager/display_manager.h"

typedef struct {
    screen_content_t** screens;     // Array of screen definitions
    uint8_t screen_count;          // Number of screens
    int8_t current_index;          // Currently displayed screen
    int8_t default_index;          // Index of the default screen
    uint32_t timeout_start;        // When current non-default screen started
    const uint32_t timeout_ms;     // timeout in ms, or cycle time if cycle_screens
    bool cycle_screens;            // Whether to cycle through screens, using timeout_ms
} mfd_config_t;

typedef struct {
    mfd_config_t** collections;     // Array of screen collections
    uint8_t collection_count;      // Number of collections
    uint8_t active_collection;     // Index of currently active collection
} mfd_state_t;

extern mfd_state_t mfd_state;      // Global state containing all collections

void mfd_init(void);
void change_collection(bool positive_increment);
void increment_screen(bool positive_increment);
