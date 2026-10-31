#include <iostream>
#include <string>
#include <vector>
#include "Memory.h"
#include "CPU.h"
#include "PPU.h"
#include "Timer.h"
#include "APU.h"
#include <SDL.h>

int main() {
    std::string romPath = "/Users/galois/CLionProjects/gameboy-emulator/roms/pokemon-red.gb";

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Game Boy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 4, 144 * 4, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    SDL_AudioSpec want{};
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    SDL_PauseAudioDevice(audioDevice, 0);

    Memory memory;
    if (!memory.loadRom(romPath)) {
        std::cerr << "Could not open ROM: " << romPath << std::endl;
        return 1;
    }

    CPU cpu(memory);
    PPU ppu(memory);
    Timer timer(memory);
    APU apu(memory);

    int frameCount = 0;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                bool pressed = event.type == SDL_KEYDOWN;
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_RIGHT) {
                    memory.setButton(0, pressed);
                }
                if (key == SDLK_LEFT) {
                    memory.setButton(1, pressed);
                }
                if (key == SDLK_UP) {
                    memory.setButton(2, pressed);
                }
                if (key == SDLK_DOWN) {
                    memory.setButton(3, pressed);
                }
                if (key == SDLK_x) {
                    memory.setButton(4, pressed);
                }
                if (key == SDLK_z) {
                    memory.setButton(5, pressed);
                }
                if (key == SDLK_BACKSPACE) {
                    memory.setButton(6, pressed);
                }
                if (key == SDLK_RETURN) {
                    memory.setButton(7, pressed);
                }
                if (key == SDLK_p) {
                    if (pressed) {
                        ppu.paletteSet = (ppu.paletteSet + 1) % 4;
                    }
                }
            }
        }

        int frameCycles = 0;
        while (!ppu.frameReady && frameCycles < 80000) {
            cpu.step();
            timer.tick(cpu.cycles);
            ppu.tick(cpu.cycles);
            apu.tick(cpu.cycles);
            frameCycles += cpu.cycles;
        }
        ppu.frameReady = false;

        if (memory.sgb.mask == 0) {
            SDL_UpdateTexture(texture, nullptr, ppu.framebuffer.data(), 160 * sizeof(uint32_t));
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_QueueAudio(audioDevice, apu.samples.data(), apu.samples.size() * sizeof(float));
        apu.samples.clear();
        while (SDL_GetQueuedAudioSize(audioDevice) > 4096 * sizeof(float)) {
            SDL_Delay(1);
        }

        frameCount += 1;
        if (frameCount % 300 == 0) {
            memory.saveGame();
        }
    }

    memory.saveGame();
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}