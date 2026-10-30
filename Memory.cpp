
#include "Memory.h"
#include <fstream>
#include <iostream>
Memory::Memory() {
    data[0xFF40] = 0x91;
    data[0xFF47] = 0xFC;
}

uint8_t Memory::read(uint16_t address) const {
    return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    if (address == 0xFF04) {
        data[0xFF04] = 0;
        return;
    }
    data[address] = value;
    if (address == 0xFF02 && value == 0x81) {
        char c = static_cast<char>(data[0xFF01]);
        serialOutput += c;
        std::cout << c << std::flush;
    }
}

void Memory::writeRaw(uint16_t address, uint8_t value) {
    data[address] = value;
}

void Memory::requestInterrupt(int bit) {
    data[0xFF0F] = data[0xFF0F] | (1 << bit);
}

bool Memory::loadRom(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    else {
        file.read(reinterpret_cast<char*>(data.data()), 0x8000);
        return true;
    }
}