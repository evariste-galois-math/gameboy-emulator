cmake_minimum_required(VERSION 4.1)
project(gameboy_emulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(gameboy_emulator main.cpp
        Registers.cpp
        Registers.h
        Memory.cpp
        Memory.h
        CPU.cpp
        CPU.h)
