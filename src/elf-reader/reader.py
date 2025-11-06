"""
Software Name: CS4WAMR
SPDX-FileCopyrightText: Copyright (c) Orange SA
SPDX-License-Identifier: MIT

This software is distributed under the MIT licence,
see the "LICENSE" file for more details or https://opensource.org/license/mit
"""

from elf_utils import parse_elf, parse_elf_section, find_symbol_position
from binary_utils import read_at_pos, write_at_pos
import argparse
import sys


def partition_lib_symbols(lib_path, symbols, filter_fnc=lambda x: False):
    lib_symbols = []
    other_symbols = []

    for symbol in symbols:
        if (
            symbol["path"].startswith(lib_path)
            and symbol["type"] != "t"
            and symbol["type"] != "T"
            and not filter_fnc(symbol)
        ):
            lib_symbols.append(symbol)
        else:
            other_symbols.append(symbol)
    return lib_symbols, other_symbols


def insert_in_group(group, symbol):
    if group["min_address"] > symbol["address"]:
        group["min_address"] = symbol["address"]
    if group["max_address"] < symbol["address"] + symbol["size"]:
        group["max_address"] = symbol["address"] + symbol["size"]
    group["symbols"].append(symbol)


def create_first_symbol_group(symbols):
    """
    max_address is not included !!!!!
    """
    group = {
        "min_address": symbols[0]["address"],
        "max_address": symbols[0]["address"] + symbols[0]["size"],
        "symbols": [symbols[0]],
    }
    for i in range(1, len(symbols)):
        symbol = symbols[i]
        insert_in_group(group, symbol)
    return group


def split_groups_with_other_symbols(groups, other_symbols):
    for other_symbol in other_symbols:
        i = 0
        while i < len(groups):
            group = groups[i]
            if (
                group["min_address"] < other_symbol["address"] < group["max_address"]
            ) or (
                group["min_address"]
                < other_symbol["address"] + other_symbol["size"]
                < group["max_address"]
            ):
                new_group = {
                    "min_address": group["max_address"],
                    "max_address": group["min_address"],
                    "symbols": [],
                }
                j = 0
                while j < len(group["symbols"]):
                    group_symbol = group["symbols"][j]
                    if (
                        other_symbol["address"]
                        >= group_symbol["address"] + group_symbol["size"]
                    ):
                        j += 1
                    else:
                        insert_in_group(new_group, group_symbol)
                        group["symbols"].remove(group_symbol)
                if len(new_group["symbols"]) > 0:
                    groups.append(new_group)
                group["max_address"] = max(
                    [symbol["address"] + symbol["size"] for symbol in group["symbols"]]
                )
            i += 1


def write_group(elf_file, offset, group):
    group_size = group["max_address"] - group["min_address"]
    write_at_pos(elf_file, offset, group["min_address"])
    write_at_pos(elf_file, offset + 4, group_size)
    return 8


def get_ignored_symbols(ignored_symbols_files, ignored_symbols_type_files):
    ignored_symbols = []
    ignored_symbols_type = []
    for ignored_symbols_file in ignored_symbols_files:
        with open(ignored_symbols_file, "r") as f:
            ignored_symbols = ignored_symbols + f.read().splitlines()
    for ignored_symbols_type_file in ignored_symbols_type_files:
        with open(ignored_symbols_type_file, "r") as f:
            ignored_symbols_type = ignored_symbols_type + f.read().splitlines()
    return ignored_symbols, ignored_symbols_type


def filter_symbols_with_list(symbols, ignored_symbols):
    return list(filter(lambda x: x["symbol"] not in ignored_symbols, symbols))


def filter_symbols_by_type(symbols, ignored_types):
    return list(filter(lambda x: x["type"] not in ignored_types, symbols))


def main(
    elf_file,
    wamr_path,
    external_env_var_name,
    external_env_count_var_name,
    ignored_symbols_files,
    ignored_symbols_type_files,
    verbose=False,
    max_group_size_check=0,
):
    ignored_symbols, ignored_symbols_types = get_ignored_symbols(
        ignored_symbols_files, ignored_symbols_type_files
    )

    sections = parse_elf_section(elf_file)

    if verbose:
        print(
            f"elf_file {elf_file}, wamr_path {wamr_path}, external_env_var_name {external_env_var_name}, external_env_count_var_name {external_env_count_var_name}"
        )
        print("sections:", sections)
    print("===================")
    symbols = parse_elf(elf_file)

    filter_function = (
        lambda x: x["type"] in ignored_symbols_types or x["symbol"] in ignored_symbols
    )
    lib_symbols, other_symbols = partition_lib_symbols(
        wamr_path, symbols, filter_function
    )

    if len(lib_symbols) == 0:
        print("Error: No lib symbol matched.")
        sys.exit(1)
        return

    groups = [create_first_symbol_group(lib_symbols)]

    split_groups_with_other_symbols(groups, other_symbols)

    external_env_var_count_offset = find_symbol_position(
        sections, other_symbols, external_env_count_var_name
    )
    external_env_var_count = read_at_pos(elf_file, external_env_var_count_offset)
    external_env_var_offset = find_symbol_position(
        sections, other_symbols, external_env_var_name
    )
    print(
        "Maximum number of environments:", external_env_var_count
    )  # , external_env_var_count_offset
    print("===================")

    if external_env_var_count < len(groups):
        print("Not enought slot for external env")
        print("len group: ", len(groups))
        sys.exit(1)
        return

    address_offset = external_env_var_offset


    max_group_size = max([group["max_address"] - group["min_address"] for group in groups])
    if max_group_size_check != 0 and max_group_size > max_group_size_check:
        print(f"The longest group size ({max_group_size}) is bigger that the limit for the maximum group size ({max_group_size_check}): {max_group_size} > {max_group_size_check}")
        sys.exit(1)
        return

    for group in groups:
        address_offset = address_offset + write_group(elf_file, address_offset, group)
        if verbose:
            print(group)
        print(f"group address: {group['min_address']} = 0x{hex(group['min_address'])}")
        print("group size:", group["max_address"] - group["min_address"])
        print("\n====================")

    write_at_pos(elf_file, external_env_var_count_offset, len(groups))
    print("number of group: ", len(groups))
    print(
        f"external_env offset: {external_env_var_offset} = 0x{hex(external_env_var_offset)}"
    )
    print(
        "max group size: ",
        max_group_size
    )
    print(
        "Sum of group size: ",
        sum([group["max_address"] - group["min_address"] for group in groups]),
    )


if __name__ == "__main__":
    p = argparse.ArgumentParser(
        description="A script to inject WAMR static variables address in variables of the given firmware. This script allows static_context_switcher to work. It works by grouping static variable in group and writing the group size and address to the firmware."
    )
    p.add_argument(
        "--elf",
        help="Firmware on which WAMR static variable location is read and on which the location are injected. The firmware must contain file origin of symbols to work.",
        default="../wasm-final-thread/bin/dwm1001/wasm-example.elf",
    )
    p.add_argument(
        "--wamr",
        default="/RIOT/build/pkg/wamr/",
        help="Absolute path to WAMR library source code",
    )
    p.add_argument(
        "--external_name",
        default="static_values",
        help="Name of variable to inject static variables group size and address.",
    )
    p.add_argument(
        "--external_count_name",
        default="static_values_count",
        help="Name of variable to read the maximum number of static variable group and to write the number of used group.",
    )
    p.add_argument(
        "--ignored-symbols-file",
        action="append",
        default=[],
        help="File containing the symbols of WAMR to be ignored when reading reading static variables. The file should contain one symbol name by line. By using the argument multiple times, multiple files can be given.",
    )
    p.add_argument(
        "--ignored-symbols-type-file",
        action="append",
        default=[],
        help="File containing the type of symbols of WAMR to be ignored when reading reading static variables. The file should contain one symbol type by line. By using the argument multiple times, multiple files can be given.",
    )
    p.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Display debug information when running the application.",
    )
    p.add_argument(
        "--max-group-size",
        default=0,
        type=int,
        help="Maximum size for group size. This value must be lower or equal to STATIC_CONTEXT_SWITCHER_MAX_STATIC_SIZE."
    )
    args = p.parse_args()
    main(
        args.elf,
        args.wamr,
        args.external_name,
        args.external_count_name,
        args.ignored_symbols_file,
        args.ignored_symbols_type_file,
        args.verbose,
        args.max_group_size
    )
