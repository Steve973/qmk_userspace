#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "display_manager/display_manager.h"

#define MAX_MENU_DEPTH 8
#define ITEMS_PER_PAGE 10
#define DEFAULT_TIMEOUT_MS 30000

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
    RULE_VALUE_EQUALS,       // Compare variable against value
    RULE_VALUE_COMPARE      // Compare with operators (>, <, etc)
} rule_type_t;

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

typedef enum {
    DISPLAY_TYPE_MESSAGE,
    DISPLAY_TYPE_INPUT,
    DISPLAY_TYPE_SELECTION,
    DISPLAY_TYPE_LIST
} display_element_type_t;

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

typedef struct {
    const char* title;
    const char* message;
    uint8_t timeout_sec;
} display_base_config_t;

typedef union {
    struct {
        const char* ok_text;
    };
    struct {
        const char* true_text;
        const char* false_text;
    };
} display_buttons_t;

typedef struct precondition_config {
    const display_base_config_t base_config;
    const display_buttons_t button;
    const char* handler;    // Function name to execute
    void* args;             // Handler-specific configuration
} precondition_config_t;

typedef struct input_config {
    input_type_t type;
    const char* title;
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
} input_config_t;

typedef struct confirm_config {
    const display_base_config_t base_config;
    const display_buttons_t buttons;
} confirm_config_t;

typedef struct result_config {
    const display_base_config_t base_config;
    const display_buttons_t button;
} result_config_t;

typedef struct postcondition_config {
    const display_base_config_t base_config;
    const display_buttons_t button;
    const char* handler;    // Function name to execute
    void* args;             // Handler-specific configuration
} postcondition_config_t;

typedef struct conditions_config {
    condition_match_t match;
    const struct condition_rule_config {
        rule_type_t type;
        union rule_data_config {
            struct value_equals_config {
                const char* variable;
                const char* value;
            } value_equals;
            struct value_compare_config {
                const char* variable;
                const char* value;
                compare_operator_t operator;
            } value_compare;
        } rule_data;
    }* rules;
    uint8_t rule_count;
} conditions_config_t;

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

/**
 * Represents a single item in the menu system with all its
 * associated configuration and behavior.
 */
typedef struct menu_item {
    // Display properties
    const char* label;
    const char* label_short;
    const char* icon;
    const char* shortcut;
    const char* help_text;
    menu_type_t type;

    // Operation configuration (for action items)
    operation_config_t operation;

    // Visibility conditions
    conditions_config_t conditions;

    // Menu hierarchy
    const struct menu_item* const* children;
    uint8_t child_count;

    // Screen item for MENU_TYPE_DISPLAY items
    const screen_content_t* screen_content;

    // Display content
    const display_content_t* display;
} menu_item_t;

// Item creation and management
menu_item_t* menu_create_item(const char* label, const char* short_label, menu_type_t type);
bool menu_add_child(menu_item_t* parent, const menu_item_t* child);
void menu_free_item(const menu_item_t* item);
