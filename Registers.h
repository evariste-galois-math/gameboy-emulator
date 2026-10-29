#ifndef GAMEBOY_EMULATOR_REGISTERS_H
#define GAMEBOY_EMULATOR_REGISTERS_H

#include <cstdint>

struct Registers {
    uint8_t a = 0x01;
    uint8_t f = 0xB0;

    uint8_t b = 0x00;
    uint8_t c = 0x13;

    uint8_t d = 0x00;
    uint8_t e = 0xD8;

    uint8_t h = 0x01;
    uint8_t l = 0x4D;

    uint16_t sp = 0xFFFE;
    uint16_t pc = 0x0100;
};

#endif
