
#include "Registers.h"

uint16_t Registers::getBC() const {
    return (b << 8) | c;
}

void Registers::setBC(uint16_t value) {
    b = value >> 8;
    c = value & 0xFF;
}

uint16_t Registers::getDE() const {
    return (d << 8) | e;
}

void Registers::setDE(uint16_t value) {
    d = value >> 8;
    e = value & 0xFF;
}

uint16_t Registers::getHL() const {
    return (h << 8) | l;
}

void Registers::setHL(uint16_t value) {
    h = value >> 8;
    l = value & 0xFF;
}

uint16_t Registers::getAF() const {
    return (a << 8) | f;
}

void Registers::setAF(uint16_t value) {
    a = value >> 8;
    f = value & 0xF0;
}

