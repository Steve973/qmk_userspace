#include <stdlib.h>
#include <string.h>
#include "logging/debug.h"
#include "menu_item.h"

menu_item_t* menu_create_item(const char* label, const char* short_label, menu_type_t type) {
    menu_item_t* item = malloc(sizeof(menu_item_t));
    if (!item) {
        dprintf("Failed to allocate menu item\n");
        return NULL;
    }

    memset(item, 0, sizeof(menu_item_t));
    item->label = label;
    item->label_short = short_label;
    item->type = type;

    return item;
}

bool menu_add_child(menu_item_t* parent, const menu_item_t* child) {
    if (!parent || !child) {
        dprintf("Invalid parent or child\n");
        return false;
    }

    // Create new array with space for one more child
    const menu_item_t* const* new_children = realloc(
        (void*)parent->children,
        (parent->child_count + 1) * sizeof(menu_item_t*)
    );

    if (!new_children) {
        dprintf("Failed to reallocate children array\n");
        return false;
    }

    // Cast is safe because we're modifying a non-const parent
    parent->children = new_children;
    ((const menu_item_t**)new_children)[parent->child_count] = child;
    parent->child_count++;

    return true;
}

void menu_free_item(const menu_item_t* item) {
    if (!item) return;

    // Free children recursively
    if (item->children) {
        for (uint8_t i = 0; i < item->child_count; i++) {
            menu_free_item(item->children[i]);
        }
        free((void*)item->children);
    }

    // Free operation data
    if (item->operation.inputs) {
        free((void*)item->operation.inputs);
    }
    if (item->operation.confirm) {
        free((void*)item->operation.confirm);
    }
    if (item->operation.result) {
        free((void*)item->operation.result);
    }

    // Free condition rules
    if (item->conditions.rules) {
        free((void*)item->conditions.rules);
    }

    free((void*)item);
}
