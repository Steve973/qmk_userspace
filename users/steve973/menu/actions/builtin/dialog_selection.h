#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "action.h"
#include "display_manager/display_manager.h"

typedef struct {
    screen_content_t* screen;        // Current screen being selected from
    uint8_t item_count;              // Number of selectable items
    int8_t current_index;            // Currently selected item (-1 if none)
    bool selection_made;             // Whether a selection was confirmed
} selection_context_t;

void init_selection(screen_content_t* screen, uint8_t selectable_count);
bool process_selection_key(uint16_t keycode, keyrecord_t* record);
int8_t get_selection(void);
void clear_selection(void);
void set_selection(int8_t index);
