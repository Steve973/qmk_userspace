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
        self.screen_content_refs = set()

    def write_header(self):
        extern_decls = "\n".join(f"extern const screen_content_t {name};"
                                for name in sorted(self.screen_content_refs))
        self.output.write(f"""
#include "menu/core/structure/menu_item.h"

// External screen content declarations
{extern_decls if self.screen_content_refs else ""}

// Generated menu structure
""")

    def write_footer(self):
        self.output.write("""
const menu_item_t* const menu_root = &menu_item_0;
""")

    def collect_screen_content_refs(self, item: MenuItem):
        """Collect all screen_content references from menu tree"""
        if item.screen_content:
            self.screen_content_refs.add(item.screen_content)
        for child in item.children:
            self.collect_screen_content_refs(child)

    def generate(self, root: MenuItem, output: TextIO):
        self.output = output
        self.collect_screen_content_refs(root)  # Collect refs first
        self.write_header()                     # Now header can use the refs
        self.generate_menu_items(root)
        self.write_footer()

    def generate_menu_items(self, item: MenuItem, parent_name: str = None):
        item_name = f"menu_item_{self.item_count}"
        self.item_count += 1

        # Collect screen_content reference if it exists
        if item.screen_content:
            self.screen_content_refs.add(item.screen_content)

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

    # Helper function to get type indicator
    @staticmethod
    def get_type_indicator(item: MenuItem) -> str:
        if item.type.value == "submenu":
            return "[S]"
        elif item.type.value == "action":
            return "[A]"
        elif item.type.value == "display":
            return "[D]"
        return "[?]"

    @staticmethod
    def generate_menu_tree(root: MenuItem, indent: int = 0, parent_is_last = []) -> str:
        """Generate tree-like visualization of menu structure"""
        MENU_WIDTH = 40
        output = []

        if indent == 0:
            output.append("Menu Structure:")
            output.append("└── " + root.label)

        # Build prefix using parent_is_last history
        prefix = " " # One space for prefix because it is preceded by the type indicator
        for i in range(indent):
            prefix += "│   " if not parent_is_last[i] else "    "

        for i, child in enumerate(root.children):
            is_last = i == len(root.children) - 1
            connector = "└── " if is_last else "├── "

            base_line = f"{CGenerator.get_type_indicator(child)}{prefix}{connector}{child.label}"
            if child.enabled_by:
                line = f"{base_line:<{MENU_WIDTH}}[{child.enabled_by}]"
            else:
                line = base_line
            output.append(line)

            if child.children:
                new_parent_is_last = parent_is_last + [is_last]
                output.extend(CGenerator.generate_menu_tree(child, indent + 1, new_parent_is_last).split('\n'))

        return "\n".join(output)
