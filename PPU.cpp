
#include "PPU.h"

static const uint32_t COLORS[4] = {
    0xFF7BBC0F,
    0xFF8BAC0F,
    0xFF306230,
    0xFF0F380F
};

PPU::PPU(Memory& memory) : memory(memory) {
}

void PPU::renderFrame() {
    uint8_t lcdc = memory.read(0xFF40);
    uint8_t scy = memory.read(0xFF42);
    uint8_t scx = memory.read(0xFF43);
    uint8_t bgp = memory.read(0xFF47);

    uint16_t mapBase = 0x9800;
    if ((lcdc & 0x08) != 0) {
        mapBase = 0x9C00;
    }
    bool unsignedTiles = (lcdc & 0x10) != 0;

    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            int bgX = (x + scx) & 0xFF;
            int bgY = (y + scy) & 0xFF;

            uint16_t mapAddress = mapBase + (bgY / 8) * 32 + (bgX / 8);
            uint8_t tileIndex = memory.read(mapAddress);

            uint16_t tileAddress = 0;
            if (unsignedTiles) {
                tileAddress = 0x8000 + tileIndex * 16;
            }
            else {
                tileAddress = 0x9000 + static_cast<int8_t>(tileIndex) * 16;
            }

            int row = bgY % 8;
            uint8_t low = memory.read(tileAddress + row * 2);
            uint8_t high = memory.read(tileAddress + row * 2 + 1);

            int bit = 7 - (bgX % 8);
            uint8_t colorIndex = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);

            uint8_t shade = (bgp >> (colorIndex * 2)) & 0x03;
            framebuffer[y * 160 + x] = COLORS[shade];
        }
    }
}