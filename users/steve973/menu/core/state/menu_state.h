#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../structure/menu_item.h"

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

// History for menu back navigation
typedef struct {
    const menu_item_t* items[MAX_MENU_DEPTH];
    int8_t depth;
} menu_history_t;

// Core menu state structure
typedef struct menu_state {
    const menu_item_t* current;      // Current menu being displayed
    uint8_t selected_index;          // Currently selected item
    uint32_t last_activity;          // For timeout tracking
    uint32_t timeout_ms;             // Configurable timeout
    bool show_shortcuts;             // Show keyboard shortcuts
    menu_history_t history;          // Navigation history

    // Operation state
    struct {
        const menu_item_t* item;     // Item being operated on
        void* input_value;           // Current input value
        void* previous_value;        // Previous value
        bool in_progress;            // Operation in progress
    } operation;
} menu_state_t;

// State management functions
void init_menu_state(void);
bool push_menu_history(const menu_item_t* item);
bool pop_menu_history(void);
uint8_t get_history_depth(void);

// State query functions
bool is_menu_active(void);
nav_context_t get_current_context(void);
bool is_shortcuts_enabled(void);
int8_t get_selected_index(void);
const menu_item_t* get_current_menu(void);
uint32_t get_menu_timeout(void);

// State modification functions
bool set_current_menu(const menu_item_t* menu);
bool set_selected_index(uint8_t index);
void set_menu_timeout(uint32_t timeout_ms);
void set_shortcuts_visible(bool visible);
