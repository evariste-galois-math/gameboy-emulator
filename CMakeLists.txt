cmake_minimum_required(VERSION 4.1)
project(gameboy_emulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(gameboy_emulator main.cpp
        Registers.cpp
        Registers.h
        Memory.cpp
        Memory.h
        CPU.cpp
        CPU.h
        PPU.cpp
        PPU.h
        Timer.cpp
        Timer.h)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
find_package(SDL2 REQUIRED)
target_link_libraries(gameboy_emulator SDL2::SDL2)