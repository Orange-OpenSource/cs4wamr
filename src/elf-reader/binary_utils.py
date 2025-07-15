"""
 Software Name: CS4WAMR
 SPDX-FileCopyrightText: Copyright (c) Orange SA
 SPDX-License-Identifier: MIT
 
 This software is distributed under the MIT licence,
 see the "LICENSE" file for more details or https://opensource.org/license/mit
"""

import struct

LITTLE_ENDIAN = "<"
BIG_ENDIAN = ">"
INT = "i"
UNSIGNED_INT = "I"

def open_file(filepath, mode="br"):
    return open(filepath, mode=mode)


def read_at_pos(filepath, position, type=UNSIGNED_INT, byte_order=LITTLE_ENDIAN):
    with open(filepath, mode="br") as file:
        file.seek(position)
        buff = file.read(4)
        result = struct.unpack(byte_order + type, buff)
    return result[0]


def write_at_pos(filepath, position, value, type=UNSIGNED_INT, byte_order=LITTLE_ENDIAN):
    with open(filepath, mode="r+b") as file:
        file.seek(position)
        file.write(struct.pack(byte_order + type, value))
    return