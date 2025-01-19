#include "quantum/logging/debug.h"
#include "deferred_exec.h"
#include "keyboards/fingerpunch/src/fp_rgb_common.h"
#include "timeout_indicator/timeout_indicator.h"
#include "../../display/menu_display.h"
#include "../navigation/input_handler.h"
#include "../navigation/menu_navigation.h"
#include "../operation/menu_operation.h"
#include "../state/menu_state.h"
#include "menu/display/menu_display.h"
#include "menu_core.h"

// External declarations
extern const menu_item_t* const menu_root;
static uint8_t menu_timeout_token = INVALID_DEFERRED_TOKEN;

/**
 * @brief Toggles the RGB matrix lighting to indicate to indicate menu mode.
 *
 * When invoked with "true", indicating that we are entering menu mode, this function
 * saves the current RGB matrix state and sets all keys to "HSV_BLUE" with a "breathing"
 * animation.  When invoked with "false", indicating that we are leaving menu mode, this
 * function restores the RGB matrix state to the saved state.
 *
 * @param enabled True to enable menu mode lighting, false to return to saved lighting.
 */
static void set_menu_mode_lighting(bool enabled) {
    #ifdef RGB_MATRIX_ENABLE
    if (enabled) {
        // Temporarily change all keys to blue and "breathing" animation
        // to indicate that we are in menu mode and the keys are "asleep"
        fp_rgb_set_hsv_and_mode(HSV_BLUE, RGB_MATRIX_BREATHING);
    } else {
        // Restore original keyboard LED colors
        fp_rgb_set_hsv_and_mode(
            fp_config.rgb_hue,
            fp_config.rgb_sat,
            fp_config.rgb_val,
            fp_config.rgb_mode
        );
    }
    #endif
}

void update_menu_activity(void) {
    timeout_indicator_reset(menu_timeout_token);
}

static void exit_menu_timeout(void) {
    dprintln("Menu timeout reached -- exiting menu mode");
    exit_menu_mode();
}

bool enter_menu_mode(void) {
    if (is_menu_active()) {
        return false;
    }

    set_menu_mode_lighting(true);

    // Display initial menu
    menu_home();

    menu_timeout_token = timeout_indicator_create(get_menu_timeout(), &exit_menu_timeout);
    dprintln("Entered menu mode");
    return true;
}

bool exit_menu_mode(void) {
    dprintln("Exiting menu mode");
    if (!is_menu_active()) {
        return false;
    }

    timeout_indicator_cancel(menu_timeout_token);
    menu_timeout_token = INVALID_DEFERRED_TOKEN;

    // Clean up any active operation
    if (is_operation_in_progress()) {
        cancel_operation(false);
    }

    // Clear all menu screens
    while (can_navigate_back()) {
        menu_return();
    }

    // Also remove the main menu screen
    remove_menu_screen(MENU_OWNER);

    // Reset menu state
    init_menu_state();

    set_menu_mode_lighting(false);

    clear_keyboard();

    dprintln("Exited menu mode");
    return true;
}

bool process_menu_record(uint16_t keycode, keyrecord_t *record) {
    if (!is_menu_active() || !record->event.pressed) return false;

    // Handle all menu-mode input
    return handle_menu_input(keycode, record);
}
