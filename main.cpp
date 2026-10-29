#include <iostream>
#include "Registers.h"

int main() {
    Registers regs;

    std::cout << std::hex;

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

    return 0;
}