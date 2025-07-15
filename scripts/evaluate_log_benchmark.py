"""
 Software Name: CS4WAMR
 SPDX-FileCopyrightText: Copyright (c) Orange SA
 SPDX-License-Identifier: MIT
 
 This software is distributed under the MIT licence,
 see the "LICENSE" file for more details or https://opensource.org/license/mit
"""

"""
Benchmark timing using these benchmark
#define BENCH_start() printf("\n!BENCH_start\n")
#define BENCH_log(name,value_int,unit) printf("\n!BENCH_log{\"name\": \"%s\", \"value\": %d, \"unit\":\"%s\"}\n", name, value_int, unit)
#define BENCH_end() printf("\n!BENCH_end\n")
"""

import argparse
import sys
import json
import serial
import statistics


def process_line(line, variable_dict, start_trigger, stop_trigger):
    if "!BENCH_start" in line:
        start_trigger = True
    if "!BENCH_end" in line:
        stop_trigger = True
    if start_trigger and not stop_trigger and "!BENCH_log" in line:
        obj = json.loads(line[line.index("!BENCH_log") + len("!BENCH_log"):])
        if "name" in obj and "value" in obj:
            if obj["name"] not in variable_dict:
                variable_dict[obj["name"]] = {"values": [], "unit": ""}
                if "unit" in obj:
                    variable_dict[obj["name"]]["unit"] = obj["unit"]
            variable_dict[obj["name"]]["values"].append(obj["value"])
        else:
            print("invalid log:", line)
    elif len(line) > 0: print(line)
    return start_trigger, stop_trigger

def summarize_values(variables):
    result = []
    for name in variables:
        values = variables[name]["values"]
        result.append({
            "name": name, 
            "count": len(values),
            "mean": statistics.mean(values), 
            "variance": statistics.variance(values), 
            "standard_deviation": statistics.stdev(values),
            "unit": variables[name]["unit"],
        })
    return result


def read_from_serial(port='/dev/ttyACM0', baud=115200):
    ser = serial.Serial(port, baud)
    variables = dict()
    start_trigger = False
    stop_trigger = False
    while not stop_trigger:
        line = ser.readline().decode().removesuffix("\n")
        start_trigger, stop_trigger = process_line(line, variables, start_trigger, stop_trigger)
    print(variables)
    ser.close()
    return variables


def unit_convert(variables, converter):
    for name in variables:
        unit = variables[name]["unit"]
        if unit in converter:
            variables[name]["values"] = list(map(converter[unit]["fn"], variables[name]["values"]))
            variables[name]["unit"] = converter[unit]["new_unit"]

def process_reference(variables):
    if "reference" not in variables:
        print("Measurement costs have not been substracted from the variables")
        return
    mean_measurement_cost = statistics.mean(variables["reference"]["values"])
    for name in variables:
        if name == "reference":
            continue
        if variables[name]["unit"] == variables["reference"]["unit"]:
            variables[name]["values"] = list(map(lambda x: x - mean_measurement_cost, variables[name]["values"]))
        else:
            print(f"Cannot remove measurement cost to {name} as it unit is different from origin")


def main():
    # To do: other value than standard deviation !!!
    port = '/dev/ttyACM0'
    baud = 115200
    cpu_tick_per_second = 64_000_000
    tick_per_us = cpu_tick_per_second / 1_000_000
    converter = {"tick": {"fn": lambda x: x/tick_per_us, "new_unit": "µs"}}

    values = read_from_serial(port, baud)
    process_reference(values)
    unit_convert(values, converter)
    result = summarize_values(values)
    print(result)

main()