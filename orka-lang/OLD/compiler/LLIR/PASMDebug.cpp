#include <iostream>

#include <LLIR/PASM.hpp>

void PASMFile::debug() {
    std::cout << ";file = \"" << name << "\"";
    std::cout << std::endl << std::endl;
    
    for (auto instr : code) {
        instr->debug();
        std::cout << std::endl;
    }
}

void PASMInstruction::debug() {
    std::cout << "  ";
    
    switch (opType) {
        case PASM::NOP: std::cout << "nop "; break;
        case PASM::LI: std::cout << "li "; break;
        case PASM::RET: std::cout << "ret "; break;
        
        default: {}
    }
    
    if (op1 != nullptr) op1->debug();
    if (op2 != nullptr) {
        std::cout << ", ";
        op2->debug();
    }
    if (op3 != nullptr) {
        std::cout << ", ";
        op3->debug();
    }
}

void PASMLabel::debug() {
    std::cout << name << ":";
}

void PASMFunc::debug() {
    std::cout << "func " << name;
}

void PASMImm::debug() {
    std::cout << val;
}

void PASMReg::debug() {
    switch (opType) {
        case PASM::R8: std::cout << "r"; break;
        case PASM::R16: std::cout << "r"; break;
        case PASM::R32: std::cout << "r"; break;
        case PASM::R64: std::cout << "r"; break;
        default: {}
    }
    
    std::cout << num;
}

