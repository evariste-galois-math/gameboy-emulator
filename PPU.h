

#ifndef GAMEBOY_EMULATOR_PPU_H
#define GAMEBOY_EMULATOR_PPU_H
#include <array>
#include <cstdint>
#include "Memory.h"

class PPU {
public:
    PPU(Memory& memory);
    void tick(int cycles);

    std::array<uint32_t, 160 * 144> framebuffer{};
    bool frameReady = false;

private:
    Memory& memory;
    int dotCounter = 0;
    int mode = 0;

    void setMode(int newMode);
    void checkLyc();
    void renderScanline(int y);
};

#endif
