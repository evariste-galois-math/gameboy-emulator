#ifndef GAMEBOY_EMULATOR_APU_H
#define GAMEBOY_EMULATOR_APU_H

#include <cstdint>
#include <vector>
#include "Memory.h"

struct SquareChannel {
    bool enabled = false;
    int timer = 0;
    int dutyStep = 0;
    int volume = 0;
    int envelopeTimer = 0;
    int lengthCounter = 0;
    int sweepTimer = 0;
    int shadowFrequency = 0;
    bool sweepEnabled = false;
};

struct WaveChannel {
    bool enabled = false;
    int timer = 0;
    int position = 0;
    int lengthCounter = 0;
};

struct NoiseChannel : SquareChannel {
    int lfsr = 0x7FFF;
};

class APU {
public:
    APU(Memory& memory);
    void tick(int cycles);

    std::vector<float> samples;

private:
    Memory& memory;
    double sampleCounter = 0.0;
    int frameSequencerCounter = 0;
    int frameSequencerStep = 0;
    float capacitorLeft = 0.0f;
    float capacitorRight = 0.0f;

    SquareChannel ch1;
    SquareChannel ch2;
    WaveChannel ch3;
    NoiseChannel ch4;

    void triggerSquare(SquareChannel& ch, uint16_t base);
    void stepSquare(SquareChannel& ch, uint16_t base, int cycles);
    void clockLength(SquareChannel& ch, uint16_t base);
    void clockEnvelope(SquareChannel& ch, uint16_t base);
    int squareOutput(SquareChannel& ch, uint16_t base);

    void triggerSweep();
    void clockSweep();
    int sweepCalculate();

    void triggerWave();
    void stepWave(int cycles);
    void clockWaveLength();
    int waveOutput();

    void triggerNoise();
    int noisePeriod();
    void stepNoise(int cycles);
    int noiseOutput();
};

#endif