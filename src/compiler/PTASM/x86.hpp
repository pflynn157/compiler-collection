#pragma once

#include "ptasm.hpp"
#include "elf.hpp"

class X86Asm {
public:
    explicit X86Asm() {}
    void assemble(AsmFile *file);
    void link();
protected:
    void assembleFunction(AsmFunction *func);
    void assembleBlock(AsmBlock *block);
private:
    Elf64File *elf;
    int lc = 0;     // Location counter
};

