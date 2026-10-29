
#include "Memory.h"
#include <fstream>

uint8_t Memory::read(uint16_t address) const {
    return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    data[address] = value;
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