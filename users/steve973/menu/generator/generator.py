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
#include "menu/core/structure/menu_item.h"

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

    @staticmethod
    def generate_menu_tree(root: MenuItem, indent: int = 0) -> str:
        """Generate tree-like visualization of menu structure"""
        MENU_WIDTH = 40  # Consistent alignment width

        output = []
        if indent == 0:
            output.append("Menu Structure:")
            output.append("└── " + root.label)
            prefix = "    "  # Initial indent for items under Main Menu
        else:
            prefix = "    " * indent

        for i, child in enumerate(root.children):
            is_last = i == len(root.children) - 1
            # Add the appropriate connector
            if is_last:
                base_line = f"{prefix}└── {child.label}"
                if child.enabled_by:
                    line = f"{base_line:<{MENU_WIDTH}}[{child.enabled_by}]"
                else:
                    line = base_line
                output.append(line)
                child_prefix = f"{prefix}    "  # Indent for last item's children
            else:
                base_line = f"{prefix}├── {child.label}"
                if child.enabled_by:
                    line = f"{base_line:<{MENU_WIDTH}}[{child.enabled_by}]"
                else:
                    line = base_line
                output.append(line)
                child_prefix = f"{prefix}│   "  # Indent for non-last item's children

            # Process children recursively
            if child.children:
                for j, grandchild in enumerate(child.children):
                    last_grandchild = j == len(child.children) - 1
                    if last_grandchild:
                        base_line = f"{child_prefix}└── {grandchild.label}"
                        if grandchild.enabled_by:
                            line = f"{base_line:<{MENU_WIDTH}}[{grandchild.enabled_by}]"
                        else:
                            line = base_line
                        output.append(line)
                    else:
                        base_line = f"{child_prefix}├── {grandchild.label}"
                        if grandchild.enabled_by:
                            line = f"{base_line:<{MENU_WIDTH}}[{grandchild.enabled_by}]"
                        else:
                            line = base_line
                        output.append(line)

                    # Add great-grandchildren if they exist
                    if grandchild.children:
                        grandchild_prefix = f"{child_prefix}    " if last_grandchild else f"{child_prefix}│   "
                        for k, great_grandchild in enumerate(grandchild.children):
                            if k == len(grandchild.children) - 1:
                                base_line = f"{grandchild_prefix}└── {great_grandchild.label}"
                                if great_grandchild.enabled_by:
                                    line = f"{base_line:<{MENU_WIDTH}}[{great_grandchild.enabled_by}]"
                                else:
                                    line = base_line
                                output.append(line)
                            else:
                                base_line = f"{grandchild_prefix}├── {great_grandchild.label}"
                                if great_grandchild.enabled_by:
                                    line = f"{base_line:<{MENU_WIDTH}}[{great_grandchild.enabled_by}]"
                                else:
                                    line = base_line
                                output.append(line)

        return "\n".join(output)
