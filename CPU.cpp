#include "CPU.h"
#include <iostream>
#include <cstdlib>

CPU::CPU(Memory& memory) : memory(memory) {
}

uint8_t CPU::fetchByte() {
    uint8_t value = memory.read(regs.pc);
    regs.pc = regs.pc + 1;
    return value;
}

void CPU::step() {
    uint8_t opcode = fetchByte();

    switch (opcode) {
        case 0x00: {
            break;
        }

        case 0x06: {
            regs.b = fetchByte();
            break;
        }

        case 0x78: {
            regs.a = regs.b;
            break;
        }

        default: {
            std::cerr << std::hex << "Unimplemented opcode 0x"
            << static_cast<int>(opcode)
            << " at 0x" << (regs.pc - 1) <<std::endl;
            std::exit(1);
        }
    }
}
