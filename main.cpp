#include <iostream>
#include <string>
#include <vector>
#include "Memory.h"
#include "CPU.h"
#include "PPU.h"
#include "Timer.h"
#include <SDL.h>

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
    Timer timer(memory);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int frameCycles = 0;
        while (!ppu.frameReady && frameCycles < 80000) {
            cpu.step();
            timer.tick(cpu.cycles);
            ppu.tick(cpu.cycles);
            frameCycles += cpu.cycles;
        }
        ppu.frameReady = false;

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