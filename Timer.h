
#ifndef GAMEBOY_EMULATOR_TIMER_H
#define GAMEBOY_EMULATOR_TIMER_H
#include "Memory.h"

class Timer {
public:
    Timer(Memory& memory);
    void tick(int cycles);

private:
    Memory& memory;
    int divCounter = 0;
    int timaCounter = 0;
};


#endif //GAMEBOY_EMULATOR_TIMER_H