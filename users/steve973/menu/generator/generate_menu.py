#!/usr/bin/env python3

import sys
from pathlib import Path

# Refrain from using bytecode caching
sys.dont_write_bytecode = True

# Get root directory containing menu package
root_dir = Path(__file__).parent.parent.parent
sys.path.append(str(root_dir))

from menu.generator.parser import parse_menu_config
from menu.generator.generator import CGenerator

def main():
    if len(sys.argv) < 5:
        print("Usage: generate_menu.py <output_path> <tree_output_path> <defines_file> <json_path> [<json_path2> ...]")
        sys.exit(1)

    output_path = Path(sys.argv[1])
    tree_output_path = Path(sys.argv[2])
    defines_file = Path(sys.argv[3])
    json_paths = [Path(p) for p in sys.argv[4:]]

    # Read QMK feature definitions
    enabled_features = set()
    with open(defines_file) as f:
        for line in f:
            for feature in line.split():
                if feature.strip():
                    enabled_features.add(feature.strip())

    # Parse and merge all JSON files
    root, _ = parse_menu_config(json_paths, enabled_features)

    # Create generator and generate tree visualization
    generator = CGenerator()
    tree_output = generator.generate_menu_tree(root)

    # Write tree to specified location
    tree_output_path.parent.mkdir(parents=True, exist_ok=True)
    tree_output_path.write_text(tree_output)

    # Generate C code
    with open(output_path, 'w') as f:
        generator.generate(root, f)

if __name__ == '__main__':
    main()
