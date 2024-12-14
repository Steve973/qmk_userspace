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
    json_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    root = parse_menu_config(json_path)
    with open(output_path, 'w') as f:
        generator = CGenerator()
        generator.generate(root, f)

if __name__ == '__main__':
    main()
