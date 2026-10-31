#include "PPU.h"

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
            windowLine = 0;
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
    uint8_t wy = memory.read(0xFF4A);
    int wx = memory.read(0xFF4B) - 7;

    uint16_t bgMap = 0x9800;
    if ((lcdc & 0x08) != 0) {
        bgMap = 0x9C00;
    }
    uint16_t winMap = 0x9800;
    if ((lcdc & 0x40) != 0) {
        winMap = 0x9C00;
    }
    bool unsignedTiles = (lcdc & 0x10) != 0;
    bool bgEnabled = (lcdc & 0x01) != 0;
    bool windowOn = bgEnabled && (lcdc & 0x20) != 0 && y >= wy && wx < 160;
    bool windowUsed = false;

    for (int x = 0; x < 160; x++) {
        uint8_t colorIndex = 0;
        if (bgEnabled) {
            if (windowOn && x >= wx) {
                colorIndex = tilePixel(winMap, x - wx, windowLine, unsignedTiles);
                windowUsed = true;
            } else {
                int bgX = (x + scx) & 0xFF;
                int bgY = (y + scy) & 0xFF;
                colorIndex = tilePixel(bgMap, bgX, bgY, unsignedTiles);
            }
        }
        lineBgIndex[x] = colorIndex;
        uint8_t shade = (bgp >> (colorIndex * 2)) & 0x03;
        framebuffer[y * 160 + x] = memory.sgb.color(x, y, shade);
    }

    if (windowUsed) {
        windowLine += 1;
    }

    if ((lcdc & 0x02) != 0) {
        renderSprites(y);
    }
}

uint8_t PPU::tilePixel(uint16_t mapBase, int px, int py, bool unsignedTiles) {
    uint16_t mapAddress = mapBase + (py / 8) * 32 + (px / 8);
    uint8_t tileIndex = memory.read(mapAddress);

    uint16_t tileAddress = 0;
    if (unsignedTiles) {
        tileAddress = 0x8000 + tileIndex * 16;
    }
    else {
        tileAddress = 0x9000 + static_cast<int8_t>(tileIndex) * 16;
    }

    int row = py % 8;
    uint8_t low = memory.read(tileAddress + row * 2);
    uint8_t high = memory.read(tileAddress + row * 2 + 1);

    int bit = 7 - (px % 8);
    return (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
}

void PPU::renderSprites(int y) {
    uint8_t lcdc = memory.read(0xFF40);
    int height = 8;
    if ((lcdc & 0x04) != 0) {
        height = 16;
    }

    int chosen[10] = {};
    int count = 0;
    for (int i = 0; i < 40 && count < 10; i++) {
        int spriteY = memory.read(0xFE00 + i * 4) - 16;
        if (y >= spriteY && y < spriteY + height) {
            chosen[count] = i;
            count += 1;
        }
    }

    bool owned[160] = {};
    int ownerX[160] = {};
    uint8_t ownerColor[160] = {};
    uint8_t ownerFlags[160] = {};

    for (int n = 0; n < count; n++) {
        uint16_t base = 0xFE00 + chosen[n] * 4;
        int spriteY = memory.read(base) - 16;
        int spriteX = memory.read(base + 1) - 8;
        uint8_t tile = memory.read(base + 2);
        uint8_t flags = memory.read(base + 3);

        int row = y - spriteY;
        if ((flags & 0x40) != 0) {
            row = height - 1 - row;
        }
        if (height == 16) {
            tile = tile & 0xFE;
        }

        uint16_t tileAddress = 0x8000 + tile * 16 + row * 2;
        uint8_t low = memory.read(tileAddress);
        uint8_t high = memory.read(tileAddress + 1);

        for (int col = 0; col < 8; col++) {
            int x = spriteX + col;
            if (x < 0 || x >= 160) {
                continue;
            }

            int bit = 7 - col;
            if ((flags & 0x20) != 0) {
                bit = col;
            }

            uint8_t colorIndex = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
            if (colorIndex == 0) {
                continue;
            }

            if (!owned[x] || spriteX < ownerX[x]) {
                owned[x] = true;
                ownerX[x] = spriteX;
                ownerColor[x] = colorIndex;
                ownerFlags[x] = flags;
            }
        }
    }

    uint8_t obp0 = memory.read(0xFF48);
    uint8_t obp1 = memory.read(0xFF49);

    for (int x = 0; x < 160; x++) {
        if (!owned[x]) {
            continue;
        }
        if ((ownerFlags[x] & 0x80) != 0 && lineBgIndex[x] != 0) {
            continue;
        }

        uint8_t palette = obp0;
        if ((ownerFlags[x] & 0x10) != 0) {
            palette = obp1;
        }
        uint8_t shade = (palette >> (ownerColor[x] * 2)) & 0x03;
        framebuffer[y * 160 + x] = memory.sgb.color(x, y, shade);
    }
}