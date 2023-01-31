#include <iostream>

#include "elf.hpp"

int main() {
    Elf64File *elf = new Elf64File("out.o");
    
    // Add _start symbol
    elf->addFunctionSymbol("_start", 0, true, false);
    
    // mov eax, 60 = b8 3c 00 00 00
    elf->addCode8(0xB8);
    elf->addCode32(60);
    
    // mov edi, 5 = bf 05 00 00 00
    elf->addCode8(0xBF);
    elf->addCode32(5);
    
    // syscall = 0f 05
    elf->addCode8(0x0F);
    elf->addCode8(0x05);
    
    elf->write();
    return 0;
}

