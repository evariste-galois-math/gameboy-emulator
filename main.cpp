#include <iostream>
#include <string>
#include <vector>
#include "Memory.h"
#include "CPU.h"
#include <SDL.h>

int runCpuTests() {
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

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game Boy",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        160 * 4,
        144 * 4,
        0
    );
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x9B, 0xBC, 0x0F, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}