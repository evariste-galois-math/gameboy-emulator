#include <iostream>
#include "Registers.h"

int main() {
    Registers regs;

    std::cout << std::hex;

    // Starting values
    std::cout << "a=" << static_cast<int>(regs.a) << std::endl;
    std::cout << "f=" << static_cast<int>(regs.f) << std::endl;
    std::cout << "b=" << static_cast<int>(regs.b) << std::endl;
    std::cout << "c=" << static_cast<int>(regs.c) << std::endl;
    std::cout << "d=" << static_cast<int>(regs.d) << std::endl;
    std::cout << "e=" << static_cast<int>(regs.e) << std::endl;
    std::cout << "h=" << static_cast<int>(regs.h) << std::endl;
    std::cout << "l=" << static_cast<int>(regs.l) << std::endl;
    std::cout << "sp=" << regs.sp << std::endl;
    std::cout << "pc=" << regs.pc << std::endl;

    // Register pair tests
    regs.setBC(0x1234);
    std::cout << "b=" << static_cast<int>(regs.b)
              << " c=" << static_cast<int>(regs.c)
              << " BC=" << regs.getBC() << std::endl;

    regs.setDE(0x5678);
    std::cout << "d=" << static_cast<int>(regs.d)
              << " e=" << static_cast<int>(regs.e)
              << " DE=" << regs.getDE() << std::endl;

    regs.setHL(0xABCD);
    std::cout << "h=" << static_cast<int>(regs.h)
              << " l=" << static_cast<int>(regs.l)
              << " HL=" << regs.getHL() << std::endl;

    regs.setAF(0xFFFF);
    std::cout << "a=" << static_cast<int>(regs.a)
              << " f=" << static_cast<int>(regs.f)
              << " AF=" << regs.getAF() << std::endl;

    // Flag tests (fresh registers, so f starts at 0xB0)
    Registers fresh;
    std::cout << "Z=" << fresh.getZ()
              << " N=" << fresh.getN()
              << " H=" << fresh.getH()
              << " C=" << fresh.getC() << std::endl;

    fresh.setZ(false);
    std::cout << "f=" << static_cast<int>(fresh.f) << std::endl;

    fresh.setN(true);
    std::cout << "f=" << static_cast<int>(fresh.f) << std::endl;

    return 0;
}