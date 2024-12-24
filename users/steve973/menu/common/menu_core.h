/* Copyright 2024 Sadek Baroudi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include "action.h"

/* Maximum depth of menu nesting allowed */
#define MAX_MENU_DEPTH 8

typedef enum {
    NAV_CONTEXT_MENU,
    NAV_CONTEXT_PRECONDITION,
    NAV_CONTEXT_INPUT,          // Input options
    NAV_CONTEXT_CONFIRMATION,   // Yes/No
    NAV_CONTEXT_ACTION,         // Execute
    NAV_CONTEXT_RESULT,         // Acknowledge
    NAV_CONTEXT_POSTCONDITION,
    NAV_CONTEXT_INVALID
} nav_context_t;

/**
 * Menu Types - Defines the behavior of a menu item
 * ACTION:   Executes a function when selected
 * SUBMENU:  Contains child menu items
 * DISPLAY:  Shows information without action
 */
typedef enum {
    MENU_TYPE_ACTION,
    MENU_TYPE_SUBMENU,
    MENU_TYPE_DISPLAY
} menu_type_t;

/**
 * Input Types - Defines how user input is collected
 * RANGE:    Numeric value within min/max bounds
 * OPTIONS:  Selection from predefined choices
 * CUSTOM:   Custom input handling
 */
typedef enum {
    INPUT_TYPE_RANGE,
    INPUT_TYPE_OPTIONS,
    INPUT_TYPE_CUSTOM
} input_type_t;

/**
 * Result Display Modes
 * TIMED:       Show result for specified duration
 * ACKNOWLEDGE: Wait for user confirmation
 */
typedef enum {
    RESULT_MODE_TIMED,
    RESULT_MODE_ACKNOWLEDGE
} result_mode_t;

/**
 * Condition Match Types
 * ALL: All conditions must be met
 * ANY: At least one condition must be met
 */
typedef enum {
    CONDITION_MATCH_ALL,
    CONDITION_MATCH_ANY
} condition_match_t;

/**
 * Rule Types for Condition Checking
 */
typedef enum {
    RULE_FEATURE_ENABLED,    // Check if QMK feature is enabled
    RULE_VALUE_EQUALS,       // Compare variable against value
    RULE_VALUE_COMPARE      // Compare with operators (>, <, etc)
} rule_type_t;

/**
 * Comparison Operators for Value Rules
 */
typedef enum {
    COMPARE_EQUALS,
    COMPARE_NOT_EQUALS,
    COMPARE_GREATER_THAN,
    COMPARE_LESS_THAN,
    COMPARE_GREATER_EQUALS,
    COMPARE_LESS_EQUALS
} compare_operator_t;

typedef enum {
    DISPLAY_TYPE_MESSAGE,
    DISPLAY_TYPE_INPUT,
    DISPLAY_TYPE_SELECTION,
    DISPLAY_TYPE_LIST
} display_element_type_t;

typedef struct {
    display_element_type_t type;
    const char* text;
    bool is_selectable;
} display_element_t;

typedef struct {
    const char* title;
    const display_element_t* elements;
    uint8_t element_count;
} display_content_t;

typedef struct precondition_config {
    const char* handler;    // Function name to execute
    const char* message;
    void* args;             // Handler-specific configuration
} precondition_config_t;

typedef struct input_config {
    input_type_t type;
    const char* prompt;     // User prompt text
    const char* default_val;
    bool wrap;              // Wrap around at min/max
    bool live_preview;      // Update as value changes
    union input_range_or_options_data {
        // Range input configuration
        struct input_range_data {
            int16_t min;
            int16_t max;
            int16_t step;
        } range;
        // Options input configuration
        struct input_options_data {
            const char* const* options;  // Array of choices
            uint8_t option_count;
        } options;
        struct input_custom_data {
            const char* handler;    // Function name to execute
            void* data;             // Handler-specific configuration
        } custom;
    } data;
} input_config_t;            // Changed from input to inputs

typedef struct confirm_config {
    const char* message;
    uint8_t timeout_sec;
    const char* true_text;   // Confirmation button text
    const char* false_text;  // Cancel button text
} confirm_config_t;

typedef struct result_config {
    const char* message;
    result_mode_t mode;
    uint8_t timeout_sec;
    const char* ok_text;
} result_config_t;

typedef struct postcondition_config {
    const char* handler;    // Function name to execute
    const char* message;
    void* args;             // Handler-specific configuration
} postcondition_config_t;

typedef struct operation_config {
    const char* action;    // Function name to execute

    const precondition_config_t* precondition;

    // Input configuration array
    const input_config_t* inputs;  // one or more inputs
    uint8_t input_count;

    // Confirmation dialog configuration
    const confirm_config_t* confirm;

    // Result display configuration
    const result_config_t* result;

    // Postcondition configuration
    const postcondition_config_t* postcondition;

    // Display content for each phase
    const display_content_t* precondition_display;
    const display_content_t* input_display;
    const display_content_t* confirm_display;
    const display_content_t* action_display;
    const display_content_t* result_display;
    const display_content_t* postcondition_display;
} operation_config_t;

typedef struct conditions_config {
    condition_match_t match;
    const struct condition_rule_config {
        rule_type_t type;
        union rule_data_config {
            const char* feature;    // Feature flag name
            struct value_equals_config {
                const char* variable;
                const char* value;
            } value_equals;
        } rule_data;
    }* rules;
    uint8_t rule_count;
} conditions_config_t;

/**
 * Menu Item Structure
 * Represents a single item in the menu system with all its
 * associated configuration and behavior.
 */
typedef struct menu_item {
    // Display properties
    const char* label;         // Full text label
    const char* label_short;   // Abbreviated label for space constrained displays
    const char* icon;          // Optional icon identifier
    const char* shortcut;      // Keyboard shortcut (KC_* format)
    const char* help_text;     // Help/description text
    menu_type_t type;          // Item type (action/submenu/display)

    // Operation configuration (for action items)
    operation_config_t operation;

    // Visibility conditions
    conditions_config_t conditions;

    // Menu hierarchy
    const struct menu_item* const* children;  // Array of child items
    uint8_t child_count;

    // Optimization for more easily generating display content
    const display_content_t* display;
} menu_item_t;

/* Public API Functions */

/**
 * Initialize the menu system
 * @return true if initialization successful
 */
bool menu_init(void);

/**
 * Exit the menu system
 */
void menu_exit(void);

/**
 * Enter current menu item (execute action or enter submenu)
 * @return true if operation successful
 */
bool menu_enter(void);

/**
 * Go back to previous menu level
 * @return true if navigation successful
 */
bool menu_back(void);

/**
 * Create a new menu item
 * @param label Full text label
 * @param short_label Abbreviated label
 * @param type Item type
 * @return Pointer to new menu item or NULL if failed
 */
menu_item_t* menu_create_item(const char* label, const char* short_label, menu_type_t type);

/**
 * Add a child item to a parent menu
 * @param parent Parent menu item
 * @param child Child menu item
 * @return true if successful
 */
bool menu_add_child(menu_item_t* parent, const menu_item_t* child);

/**
 * Free memory used by menu item and its children
 * @param item Menu item to free
 */
void menu_free_item(const menu_item_t* item);

/**
 * Check if menu system is currently active
 * @return true if menu is active
 */
bool is_menu_active(void);

/**
 * Set menu system active state
 * @param active Desired active state
 */
void set_menu_active(bool active);

/**
 * Update menu display
 */
void display_current_menu(void);

/**
 * Process keyboard input for menu navigation
 * @param keycode Key that was pressed
 * @param record Key record
 * @return true if key was handled
 */
bool process_menu_record(uint16_t keycode, keyrecord_t *record);
