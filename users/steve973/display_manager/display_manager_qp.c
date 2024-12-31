#include <stdio.h>
#include <string.h>
#include "color.h"
#include "quantum/painter/qp.h"
#include "display_manager.h"

#define DISPLAY_BUFFER_SIZE 32
static char display_buffer[DISPLAY_BUFFER_SIZE];

extern painter_device_t display;
extern painter_font_handle_t font;

void clear_display(void) {
    qp_clear(display);
}

void flush_display(void) {
    qp_flush(display);
}

uint8_t calculate_center_position(const screen_element_t* element) {
    // Calculate string length based on element type
    uint8_t length = 0;
    switch(element->type) {
        case CONTENT_TYPE_KEY_VALUE: {
            const key_value_t* kv = &element->content.key_value;
            const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;
            length = strlen(kv->label) + 2 + strlen(value); // +2 for ": "
            break;
        }
        case CONTENT_TYPE_LIST: {
            const list_item_t* item = &element->content.list_item;
            const char* text = item->is_dynamic ? item->text.get_text() : item->text.static_text;
            length = strlen(text);
            break;
        }
        case CONTENT_TYPE_IMAGE:
        case CONTENT_TYPE_CUSTOM:
            break;
    }

    return (qp_get_width(display) - length) / 2;
}

void render_title(const char* title, highlight_type_t selection) {
    if (!title) {
        return;
    }

    switch(selection) {
        case HIGHLIGHT_INVERTED:
            qp_drawtext_recolor(display, 0, 0, font, title, HSV_BLACK, HSV_WHITE);
            break;
        case HIGHLIGHT_PREFIX:
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "> %s", title);
            qp_drawtext(display, 0, 0, font, display_buffer);
            break;
        case HIGHLIGHT_GLYPH:
        case HIGHLIGHT_NONE:
        default:
            qp_drawtext(display, 0, 0, font, title);
            break;
    }
}

void render_key_value(const key_value_t* kv, uint8_t x, uint8_t y) {
    const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;
    snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%s: %s", kv->label, value);
    qp_drawtext(display, x, y * font->line_height, font, display_buffer);
}

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

void render_image(const image_t* image, uint8_t x, uint8_t y) {
    painter_image_handle_t img = qp_load_image_mem(image->data);
    if (img != NULL) {
        qp_drawimage(display, x, y, img);
        qp_close_image(img);
    }
}
