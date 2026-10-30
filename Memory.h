#ifndef GAMEBOY_EMULATOR_MEMORY_H
#define GAMEBOY_EMULATOR_MEMORY_H

#include <array>
#include <cstdint>
#include <string>
#include "Cartridge.h"

class Memory {
public:
    Memory();
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);
    void writeRaw(uint16_t address, uint8_t value);
    void requestInterrupt(int bit);
    bool loadRom(const std::string& path);
    void setButton(int key, bool pressed);
    void saveGame();
    std::string serialOutput;
    bool soundTrigger[4] = {};

private:
    std::array<uint8_t, 0x10000> data{};
    Cartridge cartridge;
    uint8_t dpad = 0x0F;
    uint8_t buttons = 0x0F;
};

#endif