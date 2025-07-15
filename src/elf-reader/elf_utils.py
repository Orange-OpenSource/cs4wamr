"""
 Software Name: CS4WAMR
 SPDX-FileCopyrightText: Copyright (c) Orange SA
 SPDX-License-Identifier: MIT
 
 This software is distributed under the MIT licence,
 see the "LICENSE" file for more details or https://opensource.org/license/mit
"""

import subprocess
import re

def parse_elf_section(elf_path):
    read_symbol_nm = subprocess.run(['readelf', '-SW', elf_path], capture_output=True, encoding="utf-8", check=True)
    regex = re.compile( r"^\s*\[\s*([0-9]+)\]"
                        r"\s*(?P<name>.\S+)?"
                        r"\s*([A-Z_]*)"
                        r"\s*(?P<address>[a-fA-F0-9]+)"
                        r"\s*(?P<offset>[a-fA-F0-9]+)"
                        r"\s*(?P<size>[a-fA-F0-9]+)"
                        )
    
    sections = []
    for line in read_symbol_nm.stdout.splitlines():
        match = regex.match(line)
        if match:
            section = match.groupdict()
            section["address"] = int(section["address"], 16)
            section["offset"] = int(section["offset"], 16)
            section["size"] = int(section["size"], 16)
            sections.append(section)
    return sections


def parse_elf(elf_path):
    read_symbol_nm = subprocess.run(['nm', '--line-numbers', "--print-size", "--numeric-sort", elf_path], capture_output=True, encoding="utf-8", check=True)

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
    return symbols


def get_file_position_from_address(sections, address):
    """
        Get the position of an address in the elf file
    """
    for section in sections:
        if section['address'] <= address and address < section['address'] + section['size']:
            return address - section['address'] + section['offset']
    return None


def find_symbol_position(sections, symbols, name):
    """
        Return the position of a symbol in a file
    """
    matched_symbols = list(filter(lambda symbol: symbol["symbol"] == name, symbols))
    if len(matched_symbols) > 1:
        print(f"Warning multiples symbols have been matched with {name}, only the first occurence will be used.")
        print("Symbols matched: ", matched_symbols)
    
    if len(matched_symbols) == 0:
        raise IndexError("Cannot find symbol in firmware: " + str(name))
    return get_file_position_from_address(sections, matched_symbols[0]["address"])
