#include <iostream>
#include <string>
#include <vector>
#include "Memory.h"
#include "CPU.h"
#include <SDL.h>
#include "PPU.h"

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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        160,
        144
    );

    Memory memory;
    if (!memory.loadRom("/Users/galois/CLionProjects/gameboy-emulator/roms/cpu_instrs/individual/06-ld r,r.gb")) {
        std::cerr << "Could not open ROM" << std::endl;
        return 1;
    }
    CPU cpu(memory);
    PPU ppu(memory);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int frameCycles = 0;
        while (frameCycles < 70224) {
            cpu.step();
            frameCycles += cpu.cycles;
        }

        ppu.renderFrame();

        SDL_UpdateTexture(texture, nullptr, ppu.framebuffer.data(), 160 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}