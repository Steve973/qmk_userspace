#include "dialog_selection.h"

static struct {
    int8_t current_selection;  // -1 if no choice made
} selection_state = {
    .current_selection = -1
};

bool process_selection_key(uint16_t keycode, keyrecord_t* record) {
    if (!record->event.pressed) return false;

    switch (keycode) {
        case KC_UP:
        case KC_DOWN:
            selection_state.current_selection = keycode == KC_UP ? 1 : -1;
            return true;
        case KC_ENTER:
            selection_state.current_selection = 0;  // or whatever index is appropriate
            return true;
        case KC_ESC:
            selection_state.current_selection = -1;
            return true;
    }
    return false;
}

int8_t get_selection(void) {
    return selection_state.current_selection;
}

void clear_selection(void) {
    selection_state.current_selection = -1;
}

void set_selection(int8_t index) {
    selection_state.current_selection = index;
}
