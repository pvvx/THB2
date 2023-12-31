#!/usr/bin/env python3
with open('bb_rom_sym_m0.txt') as symfile:
    for line in symfile:
        if line[0] == '#': continue
        sym = line.split()
        if line[0] == ';': sym = line[1:].split()
        print(f'{sym[2]} = {sym[0]};')
