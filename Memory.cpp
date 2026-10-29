
#include "Memory.h"
#include <fstream>
#include <iostream>

uint8_t Memory::read(uint16_t address) const {
    return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    data[address] = value;
    if (address == 0xFF02 && value == 0x81) {
        char c = static_cast<char>(data[0xFF01]);
        serialOutput += c;
        std::cout << c << std::flush;
    }
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