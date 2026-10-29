#ifndef GAMEBOY_EMULATOR_CPU_H
#define GAMEBOY_EMULATOR_CPU_H

#include <cstdint>
#include "Registers.h"
#include "Memory.h"

class CPU {
public:
    CPU(Memory& memory);
    void step();

    Registers regs;

private:
    Memory& memory;
    uint8_t fetchByte();
    uint16_t fetchWord();
    uint8_t getReg8(uint8_t index);
    void setReg8(uint8_t index, uint8_t value);
};


#endif //GAMEBOY_EMULATOR_CPU_H