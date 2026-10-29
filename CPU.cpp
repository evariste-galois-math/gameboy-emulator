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

uint16_t CPU::fetchWord() {
    uint8_t low = fetchByte();
    uint8_t high = fetchByte();
    return low | (high << 8);
}

void CPU::step() {
    uint8_t opcode = fetchByte();
    if (opcode >= 0x40 && opcode <= 0x7F && opcode != 0x76) {
        uint8_t dest = (opcode >> 3) & 0x07;
        uint8_t src = opcode & 0x07;
        setReg8(dest, getReg8(src));
        return;
    }

    if ((opcode & 0xC7) == 0x06) {
        uint8_t dest = (opcode >> 3) & 0x07;
        setReg8(dest, fetchByte());
        return;
    }

    if ((opcode & 0xC7) == 0x04) {
        uint8_t reg = (opcode >> 3) & 0x07;
        uint8_t value = getReg8(reg);
        uint8_t result = value + 1;
        setReg8(reg, result);
        regs.setZ(result == 0);
        regs.setN(false);
        regs.setH((value & 0x0F) == 0x0F);
        return;
    }

    if ((opcode & 0xC7) == 0x05) {
        uint8_t reg = (opcode >> 3) & 0x07;
        uint8_t value = getReg8(reg);
        uint8_t result = value - 1;
        setReg8(reg, result);
        regs.setZ(result == 0);
        regs.setN(true);
        regs.setH((value & 0x0F) == 0x00);
        return;
    }



    switch (opcode) {
        case 0x00: {
            break;
        }
        case 0xC3: {
            regs.pc = fetchWord();
            break;
        }
        case 0x21: {
            regs.setHL(fetchWord());
            break;
        }
        case 0x01: {
            regs.setBC(fetchWord());
            break;
        }
        case 0x11: {
            regs.setDE(fetchWord());
            break;
        }
        case 0x31: {
            regs.sp = fetchWord();
            break;
        }
        case 0x1A: {
            regs.a = memory.read(regs.getDE());
            break;
        }
        case 0x2A: {
            regs.a = memory.read(regs.getHL());
            regs.setHL(regs.getHL() + 1);
            break;
        }
        case 0x3A: {
            regs.a = memory.read(regs.getHL());
            regs.setHL(regs.getHL() - 1);
            break;
        }
        case 0x22: {
            memory.write(regs.getHL(), regs.a);
            regs.setHL(regs.getHL() + 1);
            break;
        }
        case 0x02: {
            memory.write(regs.getBC(), regs.a);
            break;
        }
        case 0x12: {
            memory.write(regs.getDE(), regs.a);
            break;
        }
        case 0x32: {
            memory.write(regs.getHL(), regs.a);
            regs.setHL(regs.getHL() - 1);
            break;
        }
        case 0x0A: {
            regs.a = memory.read(regs.getBC());
            break;
        }
        case 0x20: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            if (!regs.getZ()) {
                regs.pc += offset;
            }
            break;
        }
        case 0x18: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            regs.pc += offset;
            break;
        }
        case 0x28: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            if (regs.getZ()) {
                regs.pc += offset;
            }
            break;
        }
        case 0x30: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            if (!regs.getC()) {
                regs.pc += offset;
            }
            break;
        }
        case 0x38: {
            int8_t offset = static_cast<int8_t>(fetchByte());
            if  (regs.getC()) {
                regs.pc += offset;
            }
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

uint8_t CPU::getReg8(uint8_t index) {
    switch (index) {
        case 0: {
            return regs.b;
        }
        case 1: {
            return regs.c;
        }
        case 2: {
            return regs.d;
        }
        case 3: {
            return regs.e;
        }
        case 4: {
            return regs.h;
        }
        case 5: {
            return regs.l;
        }
        case 6: {
            return memory.read(regs.getHL());
        }
        default: {
            return regs.a;
        }
    }
}

void CPU::setReg8(uint8_t index, uint8_t value) {
    switch (index) {
        case 0: {
            regs.b = value;
            break;
        }
        case 1: {
            regs.c = value;
            break;
        }
        case 2: {
            regs.d = value;
            break;
        }
        case 3: {
            regs.e = value;
            break;
        }
        case 4: {
            regs.h = value;
            break;
        }
        case 5: {
            regs.l = value;
            break;
        }
        case 6: {
            memory.write(regs.getHL(), value);
            break;
        }
        default: {
            regs.a = value;
            break;
        }
    }
}
