
#ifndef GAMEBOY_EMULATOR_MEMORY_H
#define GAMEBOY_EMULATOR_MEMORY_H
#include <array>
#include <cstdint>

class Memory {
public:
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);

private:
    std::array<uint8_t, 0x10000> data{};
};

#endif //GAMEBOY_EMULATOR_MEMORY_H