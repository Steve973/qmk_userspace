#include <stdio.h>
#include <string.h>
#include "color.h"
#include "quantum/painter/qp.h"
#include "display_manager.h"

#define DISPLAY_BUFFER_SIZE 32
static char display_buffer[DISPLAY_BUFFER_SIZE];

extern painter_device_t display;
extern painter_font_handle_t font;

/**
 * @brief Clear the display.
 */
void clear_display(void) {
    qp_clear(display);
}

/**
 * @brief Flush the display to show any changes.
 */
void flush_display(void) {
    qp_flush(display);
}

/**
 * @brief Calculate the starting x-position to center text on the display.
 */
uint16_t calculate_center_xpos(const char* text) {
    uint16_t text_width = qp_textwidth(font, text);
    return (qp_get_width(display) - text_width) / 2;
}

/**
 * @brief Calculate the starting y-position for a group of text lines to center text on the display.
 */
uint16_t calculate_center_ypos(uint8_t num_lines, bool with_title) {
    uint8_t title_rows = with_title ? 2 : 0;
    uint8_t display_rows = qp_get_height(display) / font->line_height;
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
    // Get text dimensions
    uint16_t text_width = qp_textwidth(font, text);
    uint16_t line_height = font->line_height;

    // Draw the text
    if (invert) {
        qp_drawtext_recolor(display, x, y, font, text, HSV_BLACK, HSV_WHITE);
    } else {
        qp_drawtext(display, x, y, font, text);
    }

    // Draw underline
    // Start slightly inset from text edges
    uint16_t line_start_x = x + 1;
    uint16_t line_end_x = x + text_width - 1;
    uint16_t line_y = y + line_height + gap;

    // Draw multiple lines for thickness
    for (uint8_t t = 0; t < thickness; t++) {
        qp_line(display, line_start_x, line_y + t, line_end_x, line_y + t, HSV_WHITE);
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
    const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;
    snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%-10s: %s", kv->label, value);
    qp_drawtext(display, x, y * font->line_height, font, display_buffer);
}

/**
 * @brief Render a list item.
 *
 * @param item The list item to render.
 * @param x The x-coordinate of the text.
 * @param y The y-coordinate of the text.
 */
void render_list_item(const list_item_t* item, uint8_t x, uint8_t y) {
    const char* text = item->is_dynamic ? item->text.get_text() : item->text.static_text;

    switch(item->highlight_type) {
        case HIGHLIGHT_INVERTED:
            qp_drawtext_recolor(display, x, y * font->line_height, font, text, HSV_BLACK, HSV_WHITE);
            break;
        case HIGHLIGHT_PREFIX:
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%c %s",
                    item->highlight.prefix_char, text);
            qp_drawtext(display, x, y * font->line_height, font, display_buffer);
            break;
        case HIGHLIGHT_GLYPH:
        case HIGHLIGHT_NONE:
        default:
            qp_drawtext(display, x, y * font->line_height, font, text);
            break;
    }
}

/**
 * @brief Render a title with optional highlighting.
 *
 * @param title The title to render.
 * @param highlight The type of highlighting to apply.
 */
void render_image(const image_t* image, uint8_t x, uint8_t y) {
    painter_image_handle_t img = qp_load_image_mem(image->data);
    if (img != NULL) {
        qp_drawimage(display, x, y, img);
        qp_close_image(img);
    }
}
