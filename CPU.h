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
};


#endif //GAMEBOY_EMULATOR_CPU_H