#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "debug.h"
#include "deferred_exec.h"
#include "display_manager/display_manager.h"
#include "mfd.h"

/**
 * @brief Checks if a number is within a range: low inclusive, high exclusive.
 */
#define IN_RANGE(n, low, high) ((n) >= (low) && (n) < (high))

#define MFD_OWNER "mfd"

/**
 * @brief Returns the active collection configuration.
 */
static mfd_config_t* get_active_config(void) {
    if (mfd_state.active_collection >= mfd_state.collection_count) {
        dprintf("Invalid active collection: %d\n", mfd_state.active_collection);
        return NULL;
    }
    return mfd_state.collections[mfd_state.active_collection];
}

/**
 * @brief Checks the index to see if it is within bounds, wrapping if necessary.
 *
 * This function checks the provided index to see if it is within bounds. If the
 * index is out of bounds, it will wrap around to the opposite end of the screen
 * list. If the index is within bounds, it will be returned as-is.
 *
 * @param index The index to check.
 */
static uint8_t check_screen_index(int8_t index) {
    mfd_config_t* config = get_active_config();
    if (!config) return -1;
    return index >= config->screen_count ? 0 :
           index < 0 ? config->screen_count - 1 :
           index;
}

/**
 * @brief Switches to the screen at the specified index, if within range.
 *
 * This function switches the currently displayed MFD screen. Note that the
 * index must be within the range of the screen list. If the index is out of
 * bounds, the function will return without switching screens.
 *
 * @param new_index The index of the screen to switch to.
 */
static void mfd_switch_screen(int8_t new_index) {
    mfd_config_t* config = get_active_config();
    if (!config || new_index >= config->screen_count || new_index < 0) {
        return;
    }
    config->current_index = new_index;
    managed_screen_t new_screen = (managed_screen_t) {
        .owner = MFD_OWNER,
        .is_custom = false,
        .display.content = config->screens[new_index],
        .refresh_interval_ms = 200
    };
    screen_push_status_t push_status = swap_screen(new_screen);
    if (push_status != SCREEN_PUSH_SUCCESS) {
        dprintf("Failed to switch to screen %d\n", new_index);
    }
}

/**
 * @brief Cycles to the next screen after a timeout.
 *
 * This function is called after a timeout to cycle to the next screen. This
 * function is meant to be called by the deferred execution module, which is
 * indicated by its signature.
 *
 * @param trigger_time The time the timeout was triggered.
 * @param cb_arg Unused callback argument.
 *
 * @return The next scheduled (timeout) value when the screen will be switched.
 */
static uint32_t cycle_to_next_screen(uint32_t trigger_time, void* cb_arg) {
    mfd_config_t* config = get_active_config();
    if (!config) return 0;
    dprintln("Cycling to next screen");
    increment_screen(true);
    return config->cycle_screens ? config->timeout_ms : 0;
}

/**
 * @brief Increments or decrements the current screen index.
 *
 * This function increments or decrements the current screen index by one. If
 * the new index is out of bounds, it will wrap around to the opposite end of
 * the screen list.
 *
 * @param positive_increment Whether to increment (true) or decrement (false).
 */
void increment_screen(bool positive_increment) {
    mfd_config_t* config = get_active_config();
    if (!config) return;
    if (get_screen_stack_size() == 0 || strcmp(get_current_screen_owner(), MFD_OWNER) == 0) {
        int8_t new_index = config->current_index + (positive_increment ? 1 : -1);
        mfd_switch_screen(check_screen_index(new_index));
    }
}

/**
 * @brief Changes the active collection to the next or previous collection.
 *
 * This function changes the active collection to the next or previous collection
 * in the list of collections. If the active collection is already at the end of
 * the list, it will wrap around to the beginning.
 *
 * @param positive_increment Whether to increment (true) or decrement (false).
 */
void change_collection(bool positive_increment) {
    if (mfd_state.collection_count > 1) {
        mfd_state.active_collection += positive_increment ? 1 : -1;
        mfd_state.active_collection = mfd_state.active_collection % mfd_state.collection_count;
        dprintf("Changed mfd screen collection to %d\n", mfd_state.active_collection);
        mfd_switch_screen(0);
    }
}

/**
 * @brief Initializes the MFD module.
 *
 * This function initializes the MFD module. It will display the default screen
 * if one is set, otherwise it will display the logo screen.
 */
void mfd_init(void) {
    mfd_config_t* config = get_active_config();
    if (!config) return;
    if (config->cycle_screens) {
        defer_exec(10, cycle_to_next_screen, NULL);
    } else {
        mfd_switch_screen(check_screen_index(config->default_index));
    }
}
