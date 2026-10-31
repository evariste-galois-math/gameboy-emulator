#ifndef GAMEBOY_EMULATOR_SGB_H
#define GAMEBOY_EMULATOR_SGB_H

#include <cstdint>

class Memory;

class SGB {
public:
    SGB();
    void write(uint8_t value, Memory& memory);
    uint8_t joypadId() const;
    uint32_t color(int x, int y, uint8_t shade) const;

    int mask = 0;

private:
    uint8_t lastLines = 0x30;
    bool receiving = false;
    int bitCount = 0;
    uint8_t packet[16] = {};

    uint8_t buffer[112] = {};
    int bufferLength = 0;
    int packetsLeft = 0;
    int command = 0;

    int players = 1;
    int currentPlayer = 0;

    uint32_t systemPalettes[512][4] = {};
    uint32_t palettes[4][4] = {};
    uint8_t attributes[20 * 18] = {};

    void packetDone(Memory& memory);
    void runCommand(Memory& memory);
    void palSet();
    void palTrn(Memory& memory);
    void attrBlk();
    void mltReq();
};

#endif