#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SCREEN_PUSH_SUCCESS,
    SCREEN_PUSH_FAIL_STACK_FULL,
    SCREEN_PUSH_FAIL_SCREEN_NULL,
    SCREEN_PUSH_FAIL_OWNER_NULL,
    SCREEN_PUSH_FAIL_OWNER_MISMATCH,
    SCREEN_PUSH_FAIL_SCREEN_ALREADY_IN_STACK,
} screen_push_status_t;

typedef enum {
    SCREEN_POP_SUCCESS,
    SCREEN_POP_FAIL_STACK_EMPTY,
    SCREEN_POP_FAIL_OWNER_NULL,
    SCREEN_POP_FAIL_OWNER_MISMATCH,
    SCREEN_POP_FAIL_SCREEN_NOT_IN_STACK,
} screen_pop_status_t;

typedef enum {
    CONTENT_TYPE_KEY_VALUE,
    CONTENT_TYPE_LIST,
    CONTENT_TYPE_IMAGE,
    CONTENT_TYPE_CUSTOM
} content_type_t;

typedef enum {
    HIGHLIGHT_NONE,
    HIGHLIGHT_INVERTED,    // Invert the text
    HIGHLIGHT_PREFIX,      // Use prefix character (*, >, etc.)
    HIGHLIGHT_GLYPH        // Use a custom glyph/icon
} highlight_type_t;

typedef struct {
    const char* label;
    union {
        const char* static_value;
        const char* (*get_value)(void);
    } value;
    bool is_dynamic;
    const uint8_t* icon;  // Optional icon/glyph
} key_value_t;

typedef struct {
    union {
        const char* static_text;
        const char* (*get_text)(void);
    } text;
    bool is_dynamic;
    highlight_type_t highlight_type;
    union {
        char prefix_char;         // For HIGHLIGHT_PREFIX
        const uint8_t* glyph;     // For HIGHLIGHT_GLYPH
    } highlight;
    const uint8_t* icon;          // Optional icon beside text
} list_item_t;

typedef struct {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
    bool invert;                  // Whether to invert the image
} image_t;

typedef struct {
    content_type_t type;
    uint8_t x;
    uint8_t y;
    union {
        key_value_t key_value;
        list_item_t list_item;
        image_t image;
        void (*custom_render)(void);  // For full custom rendering
    } content;
} screen_element_t;

typedef struct {
    const char* title;                    // Optional screen title
    highlight_type_t title_highlight;     // In case we need to highlight the title
    screen_element_t* elements;
    uint8_t element_count;
    uint8_t highlight_index;              // Index of element to highlight
    uint8_t default_x;                    // Default starting position for elements
    uint8_t default_y;                    // that don't specify their own
    bool center_contents;                 // Whether to center elements horizontally
    int8_t (*get_highlight_index)(void);  // Function pointer to get current highlight
} screen_content_t;

// This would replace the LVGL union in your managed_screen_t:
typedef struct {
    const char* owner;
    bool is_custom;
    union {
        screen_content_t* content;    // For structured content
        void (*render)(void);         // For custom rendering
    } display;
    uint32_t refresh_interval_ms;
} managed_screen_t;

/**
 * @brief A stack of managed screens.
 *
 * The screen stack contains a stack of managed screens. The top parameter
 * specifies the index of the top screen in the stack.
 */
typedef struct {
    managed_screen_t screens[10];
    int top;
} screen_stack_t;

/**
 * @brief Adds a screen to the stop of the stack so that it can be displayed.
 *
 * This function adds a screen to the top of the stack so that it can be
 * immediately displayed.
 */
screen_push_status_t push_screen(managed_screen_t screen);

/**
 * @brief Swaps the screen at the stop of the stack with the provided screen.
 *
 * This function swaps the screen at the top of the stack, as long as the owner
 * of the current/top screen is the same owner of the provided screen.
 */
screen_push_status_t swap_screen(managed_screen_t screen);

/**
 * @brief Removes the top screen from the stack so that the next screen can be
 * displayed.
 *
 * This function removes the top screen from the stack so that the next screen
 * can be displayed. The screen owner parameter  must match the owner of the top
 * screen in the stack in order for it to be popped.
 */
screen_pop_status_t pop_screen(const char *owner);

/**
 * @brief Gets the owner of the current/top screen of the stack.
 *
 * This function returns the owner of the screen that is on the top of the stack
 * and is currently being displayed.
 */
const char* get_current_screen_owner(void);

/**
 * @brief Gets the size of the screen stack.
 *
 * This function gets the size of the screen stack.  This can be useful for the
 * determination of the reason for a failure in pushing or popping a screen.
 * For example, if pushing a screen fails, and if the stack is full, then that
 * would be the reason for the failure.  Similarly, if popping a screen fails,
 * and if the stack is empty, then that would be the reason for the failure. In
 * the case of a pop failure, if the screen stack is not empty, then the owner
 * of the screen to be popped does not match the owner supplied to the pop
 * function.
 */
uint8_t get_screen_stack_size(void);

/**
 * @brief Displays the current/top screen.
 *
 * This function displays the current/top screen.  This function should be called
 * in your main keyboard loop.  Consider using `housekeeping_task_user` for this.
 */
void show_current_screen(void);

void clear_display(void);

void render_screen_content(screen_content_t* content);

void render_title(const char* title, highlight_type_t highlight);

extern uint8_t calculate_center_position(const screen_element_t* element);

extern void render_key_value(const key_value_t* kv, uint8_t x, uint8_t y);

extern void render_list_item(const list_item_t* item, uint8_t x, uint8_t y);

extern void render_image(const image_t* image, uint8_t x, uint8_t y);
