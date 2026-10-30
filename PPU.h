

#ifndef GAMEBOY_EMULATOR_PPU_H
#define GAMEBOY_EMULATOR_PPU_H
#include <array>
#include <cstdint>
#include "Memory.h"

class PPU {
public:
    PPU(Memory& memory);
    void renderFrame();

    std::array<uint32_t, 160 * 144> framebuffer{};

private:
    Memory& memory;
};

#endif
