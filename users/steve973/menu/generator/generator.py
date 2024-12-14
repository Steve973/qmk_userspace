from typing import TextIO
from jinja2 import Environment, PackageLoader
from .models import MenuItem

class CGenerator:
    def __init__(self):
        self.env = Environment(
            loader=PackageLoader('menu.generator', 'templates'),
            trim_blocks=True,
            lstrip_blocks=True
        )
        self.template = self.env.get_template('menu_item.jinja2')
        self.item_count = 0
        self.output = None

    def write_header(self):
        self.output.write("""
#include "menu/common/menu_core.h"

// Generated menu structure
""")

    def write_footer(self):
        self.output.write("""
const menu_item_t* const menu_root = &menu_item_0;
""")

    def generate(self, root: MenuItem, output: TextIO):
        self.output = output
        self.write_header()
        self.generate_menu_items(root)
        self.write_footer()

    def generate_menu_items(self, item: MenuItem, parent_name: str = None):
        item_name = f"menu_item_{self.item_count}"
        self.item_count += 1

        # Generate children first
        child_names = []
        for child in item.children:
            child_name = self.generate_menu_items(child, item_name)
            child_names.append(child_name)

        # Render template
        self.output.write(self.template.render(
            item=item,
            name=item_name,
            child_names=child_names,
            operation_name=f"operation_{self.item_count}" if item.operation else None,
            conditions_name=f"conditions_{self.item_count}" if item.conditions else None
        ))

        return item_name
