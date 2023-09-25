//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>

#include "../llir.hpp"
#include "x86ir.hpp"

namespace LLIR {

class Amd64Writer {
public:
    explicit Amd64Writer(Module *mod);
    ~Amd64Writer();
    void compile();
    void compileInstruction(Instruction *instr, std::string prefix);
    X86Operand *compileOperand(Operand *src, Type *type, std::string prefix);
    void dump();
    void writeToFile();
    void writeToFile(std::string path);
    void build();
protected:
    std::string getSizeForType(Type *type);
    int getIntSizeForType(Type *type);
private:
    Module *mod = nullptr;
    X86File *file;
    
    int stackPos = 0;
    std::map<std::string, int> memMap;
    std::map<int, X86Reg> regMap;
    std::map<int, X86Reg> argRegMap;
};

} // end namespace LLIR
