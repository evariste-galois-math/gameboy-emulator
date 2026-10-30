

#include "Cartridge.h"
#include <fstream>
#include <iostream>

bool Cartridge::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    rom.resize(size);
    file.read(reinterpret_cast<char*>(rom.data()), size);

    type = rom[0x147];
    int ramSize = 0;
    switch (rom[0x149]) {
        case 2: {
            ramSize = 0x2000;
            break;
        }
        case 3: {
            ramSize = 0x8000;
            break;
        }
        case 4: {
            ramSize = 0x20000;
            break;
        }
        case 5: {
            ramSize = 0x10000;
            break;
        }
        default: {
            ramSize = 0;
            break;
        }
    }

    ram.resize(ramSize);

    std::cout << "Cartridge type 0x" << std::hex << static_cast<int>(type)
    << std::dec << ", ROM " << size / 1024 << " KB"
    << ", RAM " << ramSize / 1024 << " KB" << std::endl;
    return true;
}

uint8_t Cartridge::read(uint16_t address) const {
    if (address < 0x4000) {
        return rom[address];
    }

    if (address < 0x8000) {
        size_t offset = romBank * 0x4000 + (address - 0x4000);
        return rom[offset % rom.size()];
    }
    if (!ramEnabled || ram.empty() || ramBank > 3) {
        return 0xFF;
    }
    size_t offset = ramBank * 0x2000 + (address - 0xA000);
    return ram[offset % ram.size()];
}

void Cartridge::write(uint16_t address, uint8_t value) {
    bool mbc3 = (type >= 0x0F && type <= 0x13);

    if (address >= 0xA000) {
        if (ramEnabled && !ram.empty() && ramBank <= 3) {
            size_t offset = ramBank * 0x2000 + (address - 0xA000);
            ram[offset % ram.size()] = value;
        }

        return;
    }

    if (!mbc3) {
        return;
    }

    if (address < 0x2000) {
        ramEnabled = (value & 0x0F) == 0x0A;
    }
    else if (address < 0x4000) {
        romBank = value & 0x7F;
        if (romBank == 0) {
            romBank = 1;
        }
    }
    else if (address < 0x6000) {
        ramBank = value;
    }
}

