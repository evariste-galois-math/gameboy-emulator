#include "Memory.h"
#include <iostream>

Memory::Memory() {
    data[0xFF40] = 0x91;
    data[0xFF47] = 0xFC;
    data[0xFF24] = 0x77;
    data[0xFF25] = 0xF3;
    data[0xFF26] = 0xF1;
}

uint8_t Memory::read(uint16_t address) const {
    if (address < 0x8000) {
        return cartridge.read(address);
    }
    if (address >= 0xA000 && address < 0xC000) {
        return cartridge.read(address);
    }
    if (address >= 0xE000 && address < 0xFE00) {
        return data[address - 0x2000];
    }
    if (address >= 0xFEA0 && address < 0xFF00) {
        return 0xFF;
    }

    if (address == 0xFF00) {
        uint8_t select = data[0xFF00] & 0x30;
        uint8_t result = 0xC0 | select | 0x0F;
        if ((select & 0x10) == 0) {
            result = result & (0xF0 | dpad);
        }
        if ((select & 0x20) == 0) {
            result = result & (0xF0 | buttons);
        }
        if (select == 0x30) {
            result = 0xF0 | sgb.joypadId();
        }
        return result;
    }

    return data[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        cartridge.write(address, value);
        return;
    }
    if (address >= 0xA000 && address < 0xC000) {
        cartridge.write(address, value);
        return;
    }
    if (address >= 0xE000 && address < 0xFE00) {
        data[address - 0x2000] = value;
        return;
    }
    if (address >= 0xFEA0 && address < 0xFF00) {
        return;
    }

    if (address == 0xFF04) {
        data[0xFF04] = 0;
        return;
    }
    if (address == 0xFF44) {
        return;
    }
    if (address == 0xFF41) {
        data[0xFF41] = (value & 0x78) | (data[0xFF41] & 0x07);
        return;
    }
    if (address == 0xFF46) {
        uint16_t source = value << 8;
        for (int i = 0; i < 160; i++) {
            data[0xFE00 + i] = read(source + i);
        }
        data[0xFF46] = value;
        return;
    }
    if (address == 0xFF00) {
        data[0xFF00] = value & 0x30;
        sgb.write(value, *this);
        return;
    }

    data[address] = value;

    if ((value & 0x80) != 0) {
        if (address == 0xFF14) {
            soundTrigger[0] = true;
        } else if (address == 0xFF19) {
            soundTrigger[1] = true;
        } else if (address == 0xFF1E) {
            soundTrigger[2] = true;
        } else if (address == 0xFF23) {
            soundTrigger[3] = true;
        }
    }

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
    return cartridge.load(path);
}

void Memory::setButton(int key, bool pressed) {
    if (key < 4) {
        if (pressed) {
            dpad = dpad & ~(1 << key);
        }
        else {
            dpad = dpad | (1 << key);
        }
    }
    else {
        int bit = key - 4;
        if (pressed) {
            buttons = buttons & ~(1 << bit);
        }
        else {
            buttons = buttons | (1 << bit);
        }
    }

    if (pressed) {
        requestInterrupt(4);
    }
}

void Memory::saveGame() {
    cartridge.saveRam();
}