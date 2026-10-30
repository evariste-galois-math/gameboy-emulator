

#ifndef GAMEBOY_EMULATOR_CARTRIDGE_H
#define GAMEBOY_EMULATOR_CARTRIDGE_H
#include <cstdint>
#include <string>
#include <vector>

class Cartridge {
public:
    bool load(const std::string& path);
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);

private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
    uint8_t type = 0;
    int romBank = 1;
    int ramBank = 0;
    bool ramEnabled = false;
};


#endif //GAMEBOY_EMULATOR_CARTRIDGE_H