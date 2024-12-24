#include <stdio.h>
#include <string.h>
#include "oled_driver.h"
#include "display_manager.h"

// Buffer for formatting strings
#define DISPLAY_BUFFER_SIZE 32
static char display_buffer[DISPLAY_BUFFER_SIZE];

void clear_display(void) {
    oled_clear();
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

    return (OLED_DISPLAY_WIDTH - length) / 2;
}

void render_title(const char* title, highlight_type_t selection) {
    if (!title) {
        return;
    }

    oled_set_cursor(0, 0);

    switch(selection) {
        case HIGHLIGHT_INVERTED:
            oled_write(title, true);  // inverted
            break;
        case HIGHLIGHT_PREFIX:
            snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "> %s", title);
            oled_write(display_buffer, false);
            break;
        case HIGHLIGHT_GLYPH:
            // Not supported (fall-through)
        case HIGHLIGHT_NONE:
        default:
            oled_write(title, false);
            break;
    }
}

void render_key_value(const key_value_t* kv, uint8_t x, uint8_t y) {
    oled_set_cursor(x, y);

    // Get value (either static or dynamic)
    const char* value = kv->is_dynamic ? kv->value.get_value() : kv->value.static_value;

    // Format and display
    snprintf(display_buffer, DISPLAY_BUFFER_SIZE, "%s: %s", kv->label, value);
    oled_write(display_buffer, false);
}

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

void render_image(const image_t* image, uint8_t x, uint8_t y) {
    oled_set_cursor(x, y);
    oled_write_raw_P((const char*)image->data, image->width * image->height / 8);
}
