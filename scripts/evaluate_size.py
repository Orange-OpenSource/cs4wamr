"""
 Software Name: CS4WAMR
 SPDX-FileCopyrightText: Copyright (c) Orange SA
 SPDX-License-Identifier: MIT
 
 This software is distributed under the MIT licence,
 see the "LICENSE" file for more details or https://opensource.org/license/mit
"""
import argparse
import os
import subprocess
import re

def parse_elf(elf_path, print_ignored_line=False):
    read_symbol_nm = subprocess.run(['arm-none-eabi-nm', '--line-numbers', '-C', "--print-size", "--numeric-sort", elf_path], capture_output=True, encoding="utf-8", check=True)

    regex = re.compile( r"^(?P<address>[a-fA-F0-9]+) "
                        r"(?P<size>[a-fA-F0-9]+) "
                        r"(?P<type>[ABbCcDdGgiINnpRrSsTtUuVvWw\-\?]) "
                        r"(?P<symbol>\S+)\t"
                        r"(?P<path>/\S+):[0-9]+$"
                        )
    symbols = []

    for line in read_symbol_nm.stdout.splitlines():
        match = regex.match(line)
        if match:
            symbol = match.groupdict()
            symbol["address"] = int(symbol["address"], 16)
            symbol["size"] = int(symbol["size"], 16)
            symbols.append(symbol)
        elif print_ignored_line:
            print(line)
    return symbols

def filter_path(symbols, path):
    return list(filter(lambda x: x['path'].startswith(path), symbols))

def type_to_memory(type):
    if type == "t" or type == "T":
        return "text"
    if type == "d" or type == "D":
        return "data"
    if type == "b" or type == "B":
        return "bss"
    return f"Unknwown ({type})"
    

def print_size(symbols):
    size_per_memory = {}
    for symbol in symbols:
        memory = type_to_memory(symbol["type"])
        size_per_memory[memory] = symbol["size"] + size_per_memory.get(memory, 0)
    # types = set(map(lambda x: x['type'], symbols))
    # size = sum(map(lambda x: x['size'], symbols))
    for memory in size_per_memory:
        print(f"\t{memory}: {size_per_memory[memory]}")
    print(f"\t\t=> ROM usage : {size_per_memory.get("text", 0) + size_per_memory.get("data", 0)}")
    print(f"\t\t=> RAM usage : {size_per_memory.get("bss", 0) + size_per_memory.get("data", 0)}")
    
def check_overlapping_categories(categories):
    for key_i in categories:
        for key_j in categories:
            if key_i == key_j:
                continue
            if categories[key_i].startswith(categories[key_j]):
                print(f"\nPath overlapping between categories {key_i} and {key_j}: path {categories[key_i]} is included in {categories[key_i]}")
                return False
    return True

def main(elf_file, categories):
    if not check_overlapping_categories(categories):
        return
    symbols = parse_elf(elf_file)
    print("Total size for elf:")
    print_size(symbols)

    for cat in categories:
        print(f"\nCat {cat}")
        path = categories[cat]
        cat_symbols = filter_path(symbols, path)
        print_size(cat_symbols)


p = argparse.ArgumentParser()
p.add_argument("elf", default="../wasm-final-thread/bin/dwm1001/wasm-example.elf")
p.add_argument('-c', '--cat', nargs=2, action="append", metavar=('MODULE', 'PATH') )
args = p.parse_args()
categories = {}
for cat in args.cat:
    categories[cat[0]] = os.path.abspath(cat[1])
print(categories)
main(os.path.abspath(args.elf), categories)

"""
categories = {"module": module_path, "example": example_path, "wamr": wamr_path}
"""