#include <iostream>
#include "Memory.h"
#include "CPU.h"

int main() {
    Memory memory;

    if (!memory.loadRom("/Users/galois/ClionProjects/gameboy-emulator/roms/cpu_instrs/individual/06-ld r,r.gb")) {
        std::cerr << "Could not open ROM file" << std::endl;
        return 1;
    }

    CPU cpu (memory);
    while (true) {
        cpu.step();
    }
}