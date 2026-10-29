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

    if (opcode >= 0x80 && opcode <= 0xBF) {
        uint8_t op = (opcode >> 3) & 0x07;
        uint8_t src = opcode & 0x07;
        alu(op, getReg8(src));
        return;
    }

    if ((opcode & 0xC7) == 0xC6) {
        uint8_t op = (opcode >> 3) & 0x07;
        alu(op, fetchByte());
        return;
    }

    if ((opcode & 0xE7) == 0xC4) {
        uint8_t cc = (opcode >> 3) & 0x03;
        uint16_t address = fetchWord();
        if (condition(cc)) {
            push(regs.pc);
            regs.pc = address;
        }
        return;
    }

    if ((opcode & 0xE7) == 0xC2) {
        uint8_t cc = (opcode >> 3) & 0x03;
        uint16_t address = fetchWord();
        if (condition(cc)) {
            regs.pc = address;
        }
        return;
    }

    if ((opcode & 0xE7) == 0xC0) {
        uint8_t cc = (opcode >> 3) & 0x03;
        if (condition(cc)) {
            regs.pc = pop();
        }
        return;
    }

    if ((opcode & 0xC7) == 0xC7) {
        push(regs.pc);
        regs.pc = opcode & 0x38;
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
        case 0xF3: {
            ime = false;
            break;
        }
        case 0xFB: {
            ime = true;
            break;
        }
        case 0xEA: {
            memory.write(fetchWord(), regs.a);
            break;
        }
        case 0xE0: {
            memory.write(0xFF00 + fetchByte(), regs.a);
            break;
        }
        case 0xE2: {
            memory.write(0xFF00 + regs.c, regs.a);
            break;
        }
        case 0xFA: {
            regs.a = memory.read(fetchWord());
            break;
        }
        case 0xF0: {
            regs.a = memory.read(0xFF00 + fetchByte());
            break;
        }
        case 0xF2: {
            regs.a = memory.read(0xFF00 + regs.c);
            break;
        }
        case 0xC5: {
            push(regs.getBC());
            break;
        }
        case 0xC1: {
            regs.setBC(pop());
            break;
        }
        case 0xD5: {
            push(regs.getDE());
            break;
        }
        case 0xD1: {
            regs.setDE(pop());
            break;
        }
        case 0xE5: {
            push(regs.getHL());
            break;
        }
        case 0xE1: {
            regs.setHL(pop());
            break;
        }
        case 0xF5: {
            push(regs.getAF());
            break;
        }
        case 0xF1: {
            regs.setAF(pop());
            break;
        }
        case 0xCD: {
            uint16_t address = fetchWord();
            push(regs.pc);
            regs.pc = address;
            break;
        }
        case 0xC9: {
            regs.pc = pop();
            break;
        }
        case 0x23: {
            regs.setHL(regs.getHL() + 1);
            break;
        }
        case 0x2B: {
            regs.setHL(regs.getHL() - 1);
            break;
        }
        case 0x03: {
            regs.setBC(regs.getBC() + 1);
            break;
        }
        case 0x0B: {
            regs.setBC(regs.getBC() - 1);
            break;
        }
        case 0x13: {
            regs.setDE(regs.getDE() + 1);
            break;
        }
        case 0x1B: {
            regs.setDE(regs.getDE() - 1);
            break;
        }
        case 0x33: {
            regs.sp += 1;
            break;
        }
        case 0x3B: {
            regs.sp -= 1;
            break;
        }
        case 0xCB: {
            executeCB();
            break;
        }
        case 0xE8: {
            regs.sp = spPlusOffset();
            break;
        }
        case 0xF8: {
            regs.setHL(spPlusOffset());
            break;
        }
        case 0xE9: {
            regs.pc = regs.getHL();
            break;
        }
        case 0xD9: {
            regs.pc = pop();
            ime = true;
            break;
        }
        case 0x2F: {
            regs.a = ~regs.a;
            regs.setN(true);
            regs.setH(true);
            break;
        }
        case 0x37: {
            regs.setN(false);
            regs.setH(false);
            regs.setC(true);
            break;
        }
        case 0x3F: {
            regs.setN(false);
            regs.setH(false);
            regs.setC(!regs.getC());
            break;
        }
        case 0x27: {
            bool carry = regs.getC();
            if (!regs.getN()) {
                if (regs.getC() || regs.a > 0x99) {
                    regs.a += 0x60;
                    carry = true;
                }
                if (regs.getH() || (regs.a & 0x0F) > 0x09) {
                    regs.a += 0x06;
                }
            }
            else {
                if (regs.getC()) {
                    regs.a -= 0x60;
                }
                if (regs.getH()) {
                    regs.a -= 0x06;
                }
            }
            regs.setZ(regs.a == 0);
            regs.setH(false);
            regs.setC(carry);
            break;
        }
        case 0x76: {
            std::cerr << "HALT reached: needs interrupts (graphics phase)" << std::endl;
            std::exit(1);
        }
        case 0x10: {
            fetchByte();
            break;
        }
        case 0x07: {
            bool carry = (regs.a & 0x80) != 0;
            regs.a = (regs.a << 1) | (regs.a >> 7);
            regs.setZ(false);
            regs.setN(false);
            regs.setH(false);
            regs.setC(carry);
            break;
        }
        case 0x0F: {
            bool carry = (regs.a & 0x01) != 0;
            regs.a = (regs.a >> 1) | (regs.a << 7);
            regs.setZ(false);
            regs.setN(false);
            regs.setH(false);
            regs.setC(carry);
            break;
        }
        case 0x17: {
            uint8_t oldCarry = 0;
            if (regs.getC()) {
                oldCarry = 1;
            }
            bool carry = (regs.a & 0x80) != 0;
            regs.a = (regs.a << 1) | oldCarry;
            regs.setZ(false);
            regs.setN(false);
            regs.setH(false);
            regs.setC(carry);
            break;
        }
        case 0x1F: {
            uint8_t oldCarry = 0;
            if (regs.getC()) {
                oldCarry = 1;
            }
            bool carry = (regs.a & 0x01) != 0;
            regs.a = (regs.a >> 1) | (oldCarry << 7);
            regs.setZ(false);
            regs.setN(false);
            regs.setH(false);
            regs.setC(carry);
            break;
        }
        case 0x09: {
            addHL(regs.getBC());
            break;
        }
        case 0x19: {
            addHL(regs.getDE());
            break;
        }
        case 0x29: {
            addHL(regs.getHL());
            break;
        }
        case 0x39: {
            addHL(regs.sp);
            break;
        }
        case 0xF9: {
            regs.sp = regs.getHL();
            break;
        }
        case 0x08: {
            uint16_t address = fetchWord();
            memory.write(address, regs.sp & 0xFF);
            memory.write(address + 1, regs.sp >> 8);
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

void CPU::push(uint16_t value) {
    regs.sp -= 1;
    memory.write(regs.sp, value >> 8);
    regs.sp -= 1;
    memory.write(regs.sp, value & 0xFF);
}

uint16_t CPU::pop() {
    uint8_t low = memory.read(regs.sp);
    regs.sp += 1;
    uint8_t  high = memory.read(regs.sp);
    regs.sp += 1;
    return low | (high << 8);
}

void CPU::alu(uint8_t op, uint8_t value) {
    switch (op) {
        case 0: {
            uint16_t sum = regs.a + value;
            regs.setZ((sum & 0xFF) == 0);
            regs.setN(false);
            regs.setH(((regs.a & 0x0F) + (value & 0x0F)) > 0x0F);
            regs.setC(sum > 0xFF);
            regs.a = sum & 0xFF;
            break;
        }
        case 1: {
            uint8_t carry = 0;
            if (regs.getC()) {
                carry = 1;
            }
            uint16_t sum = regs.a + value + carry;
            regs.setZ((sum & 0xFF) == 0);
            regs.setN(false);
            regs.setH(((regs.a & 0x0F) + (value & 0x0F) + carry) > 0x0F);
            regs.setC(sum > 0xFF);
            regs.a = sum & 0xFF;
            break;
        }
        case 2: {
            uint8_t result = regs.a - value;
            regs.setZ(result == 0);
            regs.setN(true);
            regs.setH((regs.a  & 0x0F) < (value & 0x0F));
            regs.setC(regs.a < value);
            regs.a = result;
            break;
        }
        case 3: {
            uint8_t carry = 0;
            if (regs.getC()) {
                carry = 1;
            }
            uint8_t result = regs.a - value - carry;
            regs.setZ(result == 0);
            regs.setN(true);
            regs.setH((regs.a & 0x0F) < ((value & 0x0F) + carry));
            regs.setC(regs.a <  (value + carry));
            regs.a = result;
            break;
        }
        case 4: {
            regs.a = regs.a & value;
            regs.setZ(regs.a == 0);
            regs.setN(false);
            regs.setH(true);
            regs.setC(false);
            break;
        }
        case 5: {
            regs.a = regs.a ^ value;
            regs.setZ(regs.a == 0);
            regs.setN(false);
            regs.setH(false);
            regs.setC(false);
            break;
        }
        case 6: {
            regs.a = regs.a | value;
            regs.setZ(regs.a == 0);
            regs.setN(false);
            regs.setH(false);
            regs.setC(false);
            break;
        }
        case 7: {
            uint8_t result = regs.a - value;
            regs.setZ(result == 0);
            regs.setN(true);
            regs.setH((regs.a & 0x0F) < (value & 0x0F));
            regs.setC(regs.a < value);
            break;
        }
        default: {
            std::cerr << "Unimplemented ALU op " << static_cast<int>(op) << std::endl;
            std::exit(1);
        }
    }
}

bool CPU::condition(uint8_t cc) {
    switch (cc) {
        case 0: {
            return !regs.getZ();
        }
        case 1: {
            return regs.getZ();
        }
        case 2: {
            return !regs.getC();
        }
        default: {
            return regs.getC();
        }
    }
}

void CPU::executeCB() {
    uint8_t cb = fetchByte();
    uint8_t reg = cb & 0x07;
    uint8_t bit = (cb >> 3) & 0x07;
    uint8_t value = getReg8(reg);

    if (cb >= 0x40 && cb <= 0x7F) {
        regs.setZ((value & (1 << bit)) == 0);
        regs.setN(false);
        regs.setH(true);
        return;
    }

    if (cb >= 0x80 && cb <= 0xBF) {
        setReg8(reg, value & ~(1 << bit));
        return;
    }

    if (cb >= 0xC0) {
        setReg8(reg, value | (1 << bit));
        return;
    }

    if (cb <= 0x3F) {
        uint8_t oldCarry = 0;
        if (regs.getC()) {
            oldCarry = 1;
        }
        uint8_t result = 0;
        bool carry = false;

        switch (bit) {
            case 0: {
                carry = (value & 0x80) != 0;
                result = (value << 1) | (value >> 7);
                break;
            }
            case 1: {
                carry = (value & 0x01) != 0;
                result = (value >> 1) | (value << 7);
                break;
            }
            case 2: {
                carry = (value & 0x80) != 0;
                result = (value << 1) | oldCarry;
                break;
            }
            case 3: {
                carry = (value & 0x01) != 0;
                result = (value >> 1) | (oldCarry << 7);
                break;
            }
            case 4: {
                carry = (value & 0x80) != 0;
                result = value << 1;
                break;
            }
            case 5: {
                carry = (value & 0x01) != 0;
                result = (value >> 1) | (value & 0x80);
                break;
            }
            case 6: {
                carry = false;
                result = (value << 4) | (value >> 4);
                break;
            }
            case 7: {
                carry = (value & 0x01) != 0;
                result = value >> 1;
                break;
            }
        }

        setReg8(reg, result);
        regs.setZ(result == 0);
        regs.setN(false);
        regs.setH(false);
        regs.setC(carry);
        return;
    }
    std::cerr << std::hex << "Unimplemented CB opcode 0x"
    << static_cast<int>(cb) << std::endl;
    std::exit(1);
}

void CPU::addHL(uint16_t value) {
    uint16_t hl = regs.getHL();
    uint32_t sum = hl + value;
    regs.setN(false);
    regs.setH((hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF);
    regs.setC(sum > 0xFFFF);
    regs.setHL(sum & 0xFFFF);
}

uint16_t CPU::spPlusOffset() {
    uint8_t raw = fetchByte();
    int8_t offset = static_cast<int8_t>(raw);
    regs.setZ(false);
    regs.setN(false);
    regs.setH(((regs.sp & 0x0F) + (raw & 0x0F)) > 0x0F);
    regs.setC(((regs.sp & 0xFF) + raw) > 0xFF);
    return regs.sp + offset;
}
