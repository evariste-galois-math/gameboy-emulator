#include <iostream>
#include <string>
#include <vector>
#include "Memory.h"
#include "CPU.h"

int main() {
    std::string folder = "/Users/galois/CLionProjects/gameboy-emulator/roms/cpu_instrs/individual/";

    std::vector<std::string> tests = {
        "01-special.gb",
        "03-op sp,hl.gb",
        "04-op r,imm.gb",
        "05-op rp.gb",
        "06-ld r,r.gb",
        "07-jr,jp,call,ret,rst.gb",
        "08-misc instrs.gb",
        "09-op r,r.gb",
        "10-bit ops.gb",
        "11-op a,(hl).gb"
    };

    for (int i = 0; i < static_cast<int>(tests.size()); i++) {
        Memory memory;
        if (!memory.loadRom(folder + tests[i])) {
            std::cerr << "Could not open " << tests[i] << std::endl;
            continue;
        }

        CPU cpu(memory);

        for (int steps = 0; steps < 30000000; steps++) {
            cpu.step();
            if (steps % 10000 == 0) {
                if (memory.serialOutput.find("Passed") != std::string::npos) {
                    break;
                }
                if (memory.serialOutput.find("Failed") != std::string::npos) {
                    break;
                }
            }
        }

        std::cout << std::endl << "----------" << std::endl;
    }

    return 0;
}