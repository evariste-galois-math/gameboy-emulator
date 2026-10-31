#include "SGB.h"
#include "Memory.h"
#include <iostream>

static uint32_t toArgb(uint16_t color) {
    uint32_t r = color & 0x1F;
    uint32_t g = (color >> 5) & 0x1F;
    uint32_t b = (color >> 10) & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

SGB::SGB() {
    uint32_t defaults[4] = {0xFFF8E8C8, 0xFFD89048, 0xFFA82820, 0xFF301850};
    for (int p = 0; p < 4; p++) {
        for (int c = 0; c < 4; c++) {
            palettes[p][c] = defaults[c];
        }
    }
}

void SGB::write(uint8_t value, Memory& memory) {
    uint8_t lines = value & 0x30;
    if (lines == lastLines) {
        return;
    }
    bool p15Rose = (lastLines & 0x20) == 0 && (lines & 0x20) != 0;
    if (p15Rose && players > 1) {
        currentPlayer = (currentPlayer + 1) % players;
    }
    lastLines = lines;

    if (lines == 0x00) {
        receiving = true;
        bitCount = 0;
        for (int i = 0; i < 16; i++) {
            packet[i] = 0;
        }
        return;
    }
    if (!receiving || lines == 0x30) {
        return;
    }
    if (bitCount == 128) {
        receiving = false;
        packetDone(memory);
        return;
    }
    if (lines == 0x10) {
        packet[bitCount / 8] |= (1 << (bitCount % 8));
    }
    bitCount += 1;
}

uint8_t SGB::joypadId() const {
    return 0x0F - currentPlayer;
}

uint32_t SGB::color(int x, int y, uint8_t shade) const {
    int palette = attributes[(y / 8) * 20 + (x / 8)];
    return palettes[palette][shade];
}

void SGB::packetDone(Memory& memory) {
    if (packetsLeft == 0) {
        command = packet[0] >> 3;
        packetsLeft = packet[0] & 0x07;
        if (packetsLeft == 0) {
            packetsLeft = 1;
        }
        bufferLength = 0;
    }
    for (int i = 0; i < 16; i++) {
        buffer[bufferLength] = packet[i];
        bufferLength += 1;
    }
    packetsLeft -= 1;
    if (packetsLeft == 0) {
        runCommand(memory);
    }
}

void SGB::runCommand(Memory& memory) {
    std::cout << "SGB command 0x" << std::hex << command << std::dec << std::endl;
    if (command == 0x0A) {
        palSet();
    } else if (command == 0x0B) {
        palTrn(memory);
    } else if (command == 0x04) {
        attrBlk();
    } else if (command == 0x11) {
        mltReq();
    } else if (command == 0x17) {
        mask = buffer[1] & 0x03;
    }
}

void SGB::mltReq() {
    int mode = buffer[1] & 0x03;
    players = 1;
    if (mode == 1) {
        players = 2;
    }
    if (mode == 3) {
        players = 4;
    }
    currentPlayer = 0;
}

void SGB::palTrn(Memory& memory) {
    uint16_t base = 0x8000;
    if ((memory.read(0xFF40) & 0x10) == 0) {
        base = 0x8800;
    }
    for (int i = 0; i < 512; i++) {
        for (int c = 0; c < 4; c++) {
            uint16_t address = base + i * 8 + c * 2;
            uint16_t raw = memory.read(address) | (memory.read(address + 1) << 8);
            systemPalettes[i][c] = toArgb(raw);
        }
    }
}

void SGB::palSet() {
    for (int p = 0; p < 4; p++) {
        int index = (buffer[1 + p * 2] | (buffer[2 + p * 2] << 8)) & 0x1FF;
        for (int c = 0; c < 4; c++) {
            palettes[p][c] = systemPalettes[index][c];
        }
    }
    for (int p = 1; p < 4; p++) {
        palettes[p][0] = palettes[0][0];
    }
    if ((buffer[9] & 0x40) != 0) {
        mask = 0;
    }
}

void SGB::attrBlk() {
    int count = buffer[1] & 0x1F;
    for (int i = 0; i < count; i++) {
        int offset = 2 + i * 6;
        uint8_t control = buffer[offset];
        uint8_t choice = buffer[offset + 1];
        int x1 = buffer[offset + 2] & 0x1F;
        int y1 = buffer[offset + 3] & 0x1F;
        int x2 = buffer[offset + 4] & 0x1F;
        int y2 = buffer[offset + 5] & 0x1F;

        bool changeInside = (control & 0x01) != 0;
        bool changeBorder = (control & 0x02) != 0;
        bool changeOutside = (control & 0x04) != 0;
        int insidePalette = choice & 0x03;
        int borderPalette = (choice >> 2) & 0x03;
        int outsidePalette = (choice >> 4) & 0x03;

        if (control == 0x01) {
            changeBorder = true;
            borderPalette = insidePalette;
        }
        if (control == 0x04) {
            changeBorder = true;
            borderPalette = outsidePalette;
        }

        for (int y = 0; y < 18; y++) {
            for (int x = 0; x < 20; x++) {
                bool inRect = x >= x1 && x <= x2 && y >= y1 && y <= y2;
                bool onEdge = inRect && (x == x1 || x == x2 || y == y1 || y == y2);
                if (onEdge) {
                    if (changeBorder) {
                        attributes[y * 20 + x] = borderPalette;
                    }
                } else if (inRect) {
                    if (changeInside) {
                        attributes[y * 20 + x] = insidePalette;
                    }
                } else {
                    if (changeOutside) {
                        attributes[y * 20 + x] = outsidePalette;
                    }
                }
            }
        }
    }
}