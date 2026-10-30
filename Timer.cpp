

#include "Timer.h"

Timer::Timer(Memory& memory) : memory(memory) {
}

void Timer::tick(int cycles) {
    divCounter += cycles;
    while (divCounter >= 256) {
        divCounter -= 256;
        memory.writeRaw(0xFF04, memory.read(0xFF04) + 1);
    }

    uint8_t tac = memory.read(0xFF07);
    if ((tac & 0x04) == 0) {
        return;
    }

    int period = 1024;
    if ((tac & 0x03) == 1) {
        period = 16;
    }
    else if ((tac & 0x03) == 2) {
        period = 64;
    }
    else if ((tac & 0x03) == 3) {
        period = 256;
    }

    timaCounter += cycles;
    while (timaCounter >= period) {
        timaCounter -= period;
        uint8_t tima = memory.read(0xFF05);
        if (tima == 0xFF) {
            memory.writeRaw(0xFF05, memory.read(0xFF06));
            memory.requestInterrupt(2);
        }
        else {
            memory.writeRaw(0xFF05, tima + 1);
        }
    }
}