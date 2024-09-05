#!/usr/bin/env python3

import sys
from pathlib import Path
import argparse

# Refrain from using bytecode caching
sys.dont_write_bytecode = True

# Get root directory containing menu package
root_dir = Path(__file__).parent.parent.parent
sys.path.append(str(root_dir))

from menu.generator.parser import parse_menu_config

def parse_args():
    parser = argparse.ArgumentParser(description='Generate menu action lookup table')
    parser.add_argument('--json', required=True, help='Path to menu configuration JSON')
    parser.add_argument('--output', required=True, help='Output C file path')
    parser.add_argument('files', nargs='*', help='C source files to scan')
    return parser.parse_args()

def scan_c_file(file_path: str, action_names: set) -> dict:
    found_actions = {}
    header_path = str(Path(file_path)).replace('.c', '.h')

    with open(file_path) as f:
        for line in f:
            if 'operation_result_t' in line and '(operation_result_t prev_result, void** input_values)' in line:
                name = line.split('operation_result_t')[1].split('(')[0].strip()
                if name in action_names:
                    found_actions[name] = header_path
    return found_actions

def generate_header(output_file: str):
    header_file = output_file.replace('.c', '.h')
    with open(header_file, 'w') as f:
        f.write('#pragma once\n\n')
        f.write('#include "menu/core/operation/operation_types.h"\n\n')
        f.write('extern const size_t action_lookup_table_size;\n')
        f.write('extern const action_lookup_entry_t* const action_lookup_entries;\n')

def generate_lookup_table(output_file: str, actions: dict):
    headers = set(actions.values())

    with open(output_file, 'w') as f:
        # Write operation_types.h include
        f.write('#include "menu/core/operation/operation_types.h"\n')

        # Write includes for found actions
        for header in sorted(headers):
            f.write(f'#include "{header}"\n')
        f.write('\n')

        f.write('static const action_lookup_entry_t action_lookup_table[] = {\n')
        for name in sorted(actions.keys()):
            f.write(f'    {{"{name}", {name}}},\n')
        f.write('};\n\n')

        # Make size available externally
        f.write('const size_t action_lookup_table_size = sizeof(action_lookup_table) / sizeof(action_lookup_table[0]);\n')
        f.write('const action_lookup_entry_t* const action_lookup_entries = action_lookup_table;\n')

def main():
    args = parse_args()

    # Create output directory if it doesn't exist
    output_dir = Path(args.output).parent
    print(f"Creating directory: {output_dir}")
    output_dir.mkdir(parents=True, exist_ok=True)

    # Get action names from menu config
    root, action_names = parse_menu_config(Path(args.json))

    # Scan C files for function definitions
    found_actions = {}
    for file in args.files:
        file_actions = scan_c_file(file, action_names)
        found_actions.update(file_actions)

    # Verify all actions were found
    missing_actions = action_names - set(found_actions.keys())
    if missing_actions:
        print(f"ERROR: Could not find implementations for actions: {missing_actions}")
        sys.exit(1)

    # Generate lookup table
    output_base = args.output.replace('.c', '')
    generate_lookup_table(f"{output_base}.c", found_actions)
    generate_header(f"{output_base}.h")

if __name__ == '__main__':
    main()
