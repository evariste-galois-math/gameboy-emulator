#include "APU.h"

static const double CPU_HZ = 4194304.0;
static const double SAMPLE_RATE = 48000.0;
static const double CYCLES_PER_SAMPLE = CPU_HZ / SAMPLE_RATE;

static const uint8_t DUTY[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0}
};

APU::APU(Memory& memory) : memory(memory) {
}

void APU::tick(int cycles) {
    if (memory.soundTrigger[0]) {
        memory.soundTrigger[0] = false;
        triggerSquare(ch1, 0xFF11);
        triggerSweep();
    }
    if (memory.soundTrigger[1]) {
        memory.soundTrigger[1] = false;
        triggerSquare(ch2, 0xFF16);
    }

    stepSquare(ch1, 0xFF11, cycles);
    stepSquare(ch2, 0xFF16, cycles);

    frameSequencerCounter += cycles;
    while (frameSequencerCounter >= 8192) {
        frameSequencerCounter -= 8192;
        if (frameSequencerStep % 2 == 0) {
            clockLength(ch1, 0xFF11);
            clockLength(ch2, 0xFF16);
        }
        if (frameSequencerStep == 2 || frameSequencerStep == 6) {
            clockSweep();
        }
        if (frameSequencerStep == 7) {
            clockEnvelope(ch1, 0xFF11);
            clockEnvelope(ch2, 0xFF16);
        }
        frameSequencerStep = (frameSequencerStep + 1) % 8;
    }

    sampleCounter += cycles;
    while (sampleCounter >= CYCLES_PER_SAMPLE) {
        sampleCounter -= CYCLES_PER_SAMPLE;

        uint8_t nr50 = memory.read(0xFF24);
        uint8_t nr51 = memory.read(0xFF25);
        bool powered = (memory.read(0xFF26) & 0x80) != 0;

        float left = 0.0f;
        float right = 0.0f;
        if (powered) {
            float out1 = squareOutput(ch1, 0xFF11) / 15.0f;
            float out2 = squareOutput(ch2, 0xFF16) / 15.0f;
            if ((nr51 & 0x10) != 0) {
                left += out1;
            }
            if ((nr51 & 0x01) != 0) {
                right += out1;
            }
            if ((nr51 & 0x20) != 0) {
                left += out2;
            }
            if ((nr51 & 0x02) != 0) {
                right += out2;
            }
            left = left * (((nr50 >> 4) & 0x07) + 1) / 32.0f;
            right = right * ((nr50 & 0x07) + 1) / 32.0f;
        }

        float outLeft = left - capacitorLeft;
        capacitorLeft = left - outLeft * 0.996f;
        float outRight = right - capacitorRight;
        capacitorRight = right - outRight * 0.996f;

        samples.push_back(outLeft);
        samples.push_back(outRight);
    }
}

void APU::triggerSquare(SquareChannel& ch, uint16_t base) {
    uint8_t nrx1 = memory.read(base);
    uint8_t nrx2 = memory.read(base + 1);
    ch.enabled = (nrx2 & 0xF8) != 0;
    ch.lengthCounter = 64 - (nrx1 & 0x3F);
    ch.volume = nrx2 >> 4;
    ch.envelopeTimer = nrx2 & 0x07;
}

void APU::stepSquare(SquareChannel& ch, uint16_t base, int cycles) {
    int frequency = memory.read(base + 2) | ((memory.read(base + 3) & 0x07) << 8);
    int period = (2048 - frequency) * 4;
    ch.timer -= cycles;
    while (ch.timer <= 0) {
        ch.timer += period;
        ch.dutyStep = (ch.dutyStep + 1) % 8;
    }
}

void APU::clockLength(SquareChannel& ch, uint16_t base) {
    bool lengthEnabled = (memory.read(base + 3) & 0x40) != 0;
    if (lengthEnabled && ch.lengthCounter > 0) {
        ch.lengthCounter -= 1;
        if (ch.lengthCounter == 0) {
            ch.enabled = false;
        }
    }
}

void APU::clockEnvelope(SquareChannel& ch, uint16_t base) {
    uint8_t nrx2 = memory.read(base + 1);
    int period = nrx2 & 0x07;
    if (period == 0) {
        return;
    }
    ch.envelopeTimer -= 1;
    if (ch.envelopeTimer <= 0) {
        ch.envelopeTimer = period;
        bool up = (nrx2 & 0x08) != 0;
        if (up && ch.volume < 15) {
            ch.volume += 1;
        }
        if (!up && ch.volume > 0) {
            ch.volume -= 1;
        }
    }
}

int APU::squareOutput(SquareChannel& ch, uint16_t base) {
    if ((memory.read(base + 1) & 0xF8) == 0) {
        ch.enabled = false;
    }
    if (!ch.enabled) {
        return 0;
    }
    int duty = memory.read(base) >> 6;
    if (DUTY[duty][ch.dutyStep] == 1) {
        return ch.volume;
    }
    return 0;
}

int APU::sweepCalculate() {
    uint8_t nr10 = memory.read(0xFF10);
    int shift = nr10 & 0x07;
    int change = ch1.shadowFrequency >> shift;
    int result = 0;
    if ((nr10 & 0x08) != 0) {
        result = ch1.shadowFrequency - change;
    } else {
        result = ch1.shadowFrequency + change;
    }
    if (result > 2047) {
        ch1.enabled = false;
    }
    return result;
}

void APU::triggerSweep() {
    uint8_t nr10 = memory.read(0xFF10);
    int period = (nr10 >> 4) & 0x07;
    int shift = nr10 & 0x07;
    ch1.shadowFrequency = memory.read(0xFF13) | ((memory.read(0xFF14) & 0x07) << 8);
    ch1.sweepTimer = period;
    if (ch1.sweepTimer == 0) {
        ch1.sweepTimer = 8;
    }
    ch1.sweepEnabled = (period != 0) || (shift != 0);
    if (shift != 0) {
        sweepCalculate();
    }
}

void APU::clockSweep() {
    ch1.sweepTimer -= 1;
    if (ch1.sweepTimer > 0) {
        return;
    }

    uint8_t nr10 = memory.read(0xFF10);
    int period = (nr10 >> 4) & 0x07;
    int shift = nr10 & 0x07;
    ch1.sweepTimer = period;
    if (ch1.sweepTimer == 0) {
        ch1.sweepTimer = 8;
    }
    if (!ch1.sweepEnabled || period == 0) {
        return;
    }

    int newFrequency = sweepCalculate();
    if (newFrequency <= 2047 && shift != 0) {
        ch1.shadowFrequency = newFrequency;
        memory.writeRaw(0xFF13, newFrequency & 0xFF);
        memory.writeRaw(0xFF14, (memory.read(0xFF14) & 0xF8) | (newFrequency >> 8));
        sweepCalculate();
    }
}