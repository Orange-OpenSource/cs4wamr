import re

fill_size = {}
regex_section = re.compile( r"^ \.(?P<section>[a-zA-Z]+)[\.\s]")
regex_fill = re.compile( r"^ \*fill\*\s+0x(?P<address>[a-fA-F0-9]+)\s+0x(?P<size>[a-fA-F0-9]+)")
section_set = set()

current_section = ""
with open("../examples/evaluation-examples/evaluation-minimal-logic-single-no-thread-mem/bin/dwm1001/wasm-example.map") as f:
    for line in f:
        section_match = regex_section.match(line)
        if section_match:
            section_set.add(section_match["section"])
            if section_match["section"] in ["text", "rodata", "data", "bss"]:
                current_section = section_match["section"]
        fill_match = regex_fill.match(line)
        if fill_match:
            print(current_section, fill_match["size"])
            fill_size[current_section] = fill_size.get(current_section, 0) + int(fill_match["size"], 16)
print("fill in the different sections", fill_size)
print("sections detected: ", section_set)