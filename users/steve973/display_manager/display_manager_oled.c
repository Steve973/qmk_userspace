#include <stdio.h>
#include <string.h>
#include "oled_driver.h"
#include "display_manager.h"

#define OLED_CHAR_WIDTH 6

#define DISPLAY_BUFFER_SIZE 32
static char display_buffer[DISPLAY_BUFFER_SIZE];

/**
 * @brief Clear the display.
 */
void clear_display(void) {
    oled_clear();
}

/**
 * @brief Flush the display to show any changes.
 */
void flush_display() {
    oled_render_dirty(false);
}

/**
 * @brief Calculate the starting x-position to center text on the display.
 */
uint16_t calculate_center_xpos(const char* text) {
    uint16_t text_width = strlen(text);
    return (OLED_DISPLAY_WIDTH - text_width) / 2;
}

/**
 * @brief Calculate the starting y-position for a group of text lines to center text on the display.
 */
uint16_t calculate_center_ypos(uint8_t num_lines, bool with_title) {
    uint8_t title_rows = with_title ? 2 : 0;
    uint8_t display_rows = OLED_DISPLAY_HEIGHT / OLED_FONT_HEIGHT;
    uint8_t remaining_rows = display_rows - title_rows;
    uint8_t min_line = title_rows;
    uint8_t start_line = title_rows + ((remaining_rows - num_lines) / 2);
    return MAX(start_line, min_line);
}

/**
 * @brief Render text with an underline.
 *
 * @param text The text to render.
 * @param x The x-coordinate of the text.
 * @param y The y-coordinate of the text.
 * @param gap The number of pixels between the text and the underline.
 * @param thickness The thickness of the underline in pixels.
 * @param invert Whether to invert the text color.
 */
void render_underlined_text_adv(const char* text, uint8_t x, uint8_t y, uint8_t gap, uint8_t thickness, bool invert) {
    // Draw the text
    oled_set_cursor(x, y);
    oled_write(text, invert);

    uint8_t text_length = strlen(text);
    uint8_t line_length = (text_length * OLED_CHAR_WIDTH) - 2;  // slightly shorter
    uint8_t start_x = (x * OLED_CHAR_WIDTH) + 1;  // indent by 1 pixel
    uint8_t underline_y = ((y + 1) * 8) + gap;  // gap pixels below character

    // Draw multiple pixel rows for thickness
    for (uint8_t t = 0; t < thickness; t++) {
        for (uint8_t i = 0; i < line_length; i++) {
            oled_write_pixel(start_x + i, underline_y + t, true);
        }
    }
}

/**
 * @brief Render text with a 1-pixel underline.
 *
 * @param text The text to render.
 * @param x The x-coordinate of the text.
 * @param y The y-coordinate of the text.
 */
void render_underlined_text(const char* text, uint8_t x, uint8_t y) {
    render_underlined_text_adv(text, x, y, 2, 1, false);
}

/**
 * @brief Render a key/value pair like "key: value".
 *
 * @param kv The key/value pair to render.
 * @param x The x-coordinate of the text.
 * @param y The y-coordinate of the text.
 */
void render_key_value(const key_value_t* kv, uint8_t x, uint8_t y) {
    oled_set_cursor(x, y);

    // Get value (either static or dynamic)
    const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;

    // Format and display
    snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%-10s: %s", kv->label, value);
    oled_write(display_buffer, false);
}

/**
 * @brief Render a list item.
 *
 * @param item The list item to render.
 * @param x The x-coordinate of the text.
 * @param y The y-coordinate of the text.
 */
void render_list_item(const list_item_t* item, uint8_t x, uint8_t y) {
    oled_set_cursor(x, y);

    const char* text = item->is_dynamic ? item->text.get_text() : item->text.static_text;

    switch(item->highlight_type) {
        case HIGHLIGHT_INVERTED:
            oled_write(text, true);  // inverted
            break;
        case HIGHLIGHT_PREFIX:
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%c %s",
                    item->highlight.prefix_char, text);
            oled_write(display_buffer, false);
            break;
        case HIGHLIGHT_GLYPH:
            // Not supported (fall-through)
        case HIGHLIGHT_NONE:
        default:
            oled_write(text, false);
            break;
    }
}

/**
 * @brief Render an image on the display.
 *
 * @param image The image to render.
 * @param x The x-coordinate of the image.
 * @param y The y-coordinate of the image.
 */
void render_image(const image_t* image, uint8_t x, uint8_t y) {
    oled_set_cursor(x, y);
    oled_write_raw_P((const char*)image->data, image->width * image->height / 8);
}
