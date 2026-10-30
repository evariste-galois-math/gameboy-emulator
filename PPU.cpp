#include "PPU.h"

static const uint32_t COLORS[4] = {
    0xFFE0F8D0,
    0xFF88C070,
    0xFF346856,
    0xFF081820
};

PPU::PPU(Memory& memory) : memory(memory) {
}

void PPU::tick(int cycles) {
    uint8_t lcdc = memory.read(0xFF40);
    if ((lcdc & 0x80) == 0) {
        dotCounter = 0;
        memory.writeRaw(0xFF44, 0);
        setMode(0);
        return;
    }

    dotCounter += cycles;
    uint8_t ly = memory.read(0xFF44);

    if (ly < 144) {
        if (dotCounter < 80) {
            setMode(2);
        } else if (dotCounter < 252) {
            setMode(3);
        } else {
            if (mode == 3) {
                renderScanline(ly);
            }
            setMode(0);
        }
    }

    if (dotCounter >= 456) {
        dotCounter -= 456;
        ly = ly + 1;
        if (ly > 153) {
            ly = 0;
        }
        memory.writeRaw(0xFF44, ly);
        checkLyc();

        if (ly == 144) {
            setMode(1);
            memory.requestInterrupt(0);
            frameReady = true;
        } else if (ly < 144) {
            setMode(2);
        }
    }
}

void PPU::setMode(int newMode) {
    if (mode == newMode) {
        return;
    }
    mode = newMode;

    uint8_t stat = memory.read(0xFF41);
    stat = (stat & 0xFC) | newMode;
    memory.writeRaw(0xFF41, stat);

    if (newMode == 0 && (stat & 0x08) != 0) {
        memory.requestInterrupt(1);
    } else if (newMode == 1 && (stat & 0x10) != 0) {
        memory.requestInterrupt(1);
    } else if (newMode == 2 && (stat & 0x20) != 0) {
        memory.requestInterrupt(1);
    }
}

void PPU::checkLyc() {
    uint8_t stat = memory.read(0xFF41);
    if (memory.read(0xFF44) == memory.read(0xFF45)) {
        stat = stat | 0x04;
        if ((stat & 0x40) != 0) {
            memory.requestInterrupt(1);
        }
    } else {
        stat = stat & ~0x04;
    }
    memory.writeRaw(0xFF41, stat);
}

void PPU::renderScanline(int y) {
    uint8_t lcdc = memory.read(0xFF40);
    uint8_t scy = memory.read(0xFF42);
    uint8_t scx = memory.read(0xFF43);
    uint8_t bgp = memory.read(0xFF47);

    uint16_t mapBase = 0x9800;
    if ((lcdc & 0x08) != 0) {
        mapBase = 0x9C00;
    }
    bool unsignedTiles = (lcdc & 0x10) != 0;

    for (int x = 0; x < 160; x++) {
        if ((lcdc & 0x01) == 0) {
            framebuffer[y * 160 + x] = COLORS[0];
            continue;
        }

        int bgX = (x + scx) & 0xFF;
        int bgY = (y + scy) & 0xFF;

        uint16_t mapAddress = mapBase + (bgY / 8) * 32 + (bgX / 8);
        uint8_t tileIndex = memory.read(mapAddress);

        uint16_t tileAddress = 0;
        if (unsignedTiles) {
            tileAddress = 0x8000 + tileIndex * 16;
        } else {
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