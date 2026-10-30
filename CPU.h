#ifndef GAMEBOY_EMULATOR_CPU_H
#define GAMEBOY_EMULATOR_CPU_H

#include <cstdint>
#include "Registers.h"
#include "Memory.h"

class CPU {
public:
    CPU(Memory& memory);
    void step();
    int cycles = 0;
    Registers regs;

private:
    Memory& memory;
    uint8_t fetchByte();
    uint16_t fetchWord();
    uint8_t getReg8(uint8_t index);
    void setReg8(uint8_t index, uint8_t value);
    bool ime = false;
    void push(uint16_t value);
    uint16_t pop();
    void alu(uint8_t op, uint8_t value);
    bool condition(uint8_t cc);
    void executeCB();
    void addHL(uint16_t value);
    uint16_t spPlusOffset();
    bool halted = false;
    int eiDelay = 0;
    void executeInstruction();
};


#endif //GAMEBOY_EMULATOR_CPU_H