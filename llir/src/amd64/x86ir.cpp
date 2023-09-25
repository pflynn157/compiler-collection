//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <amd64/x86ir.hpp>

namespace LLIR {

std::string X86File::print(AsmType type) {
    std::string file = "";
    if (type == AsmType::GAS) {
        file += ".intel_syntax noprefix\n";
        
        // Data
        file += "\n";
        file += ".data\n";
        for (X86Data *ln : data) {
            file += ln->print() + "\n";
        }
        
        // Code
        file += "\n";
        file += ".text\n";
        
        for (X86Instr *ln : code) {
            switch (ln->getType()) {
                case X86Type::Label:
                case X86Type::Extern:
                case X86Type::Global: break;
                
                default: file += "  ";
            }
            file += ln->print() + "\n";
        }
    }
    
    return file;
}

std::string X86GlobalFunc::print() {
    std::string ret = "\n";
    ret += ".globl " + name + "\n";
    ret += ".type " + name + ",@function\n";
    ret += name + ":";
    return ret;
}

std::string X86Push::print() {
    return "push " + op1->print();
}

std::string X86Mov::print() {
    return "mov " + op1->print() + ", " + op2->print();
}

std::string X86Movsx::print() {
    return "movsx " + op1->print() + ", " + op2->print();
}

std::string X86Lea::print() {
    return "lea " + op1->print() + ", " + op2->print();
}

std::string X86Add::print() {
    return "add " + op1->print() + ", " + op2->print();
}

std::string X86Sub::print() {
    return "sub " + op1->print() + ", " + op2->print();
}

std::string X86And::print() {
    return "and " + op1->print() + ", " + op2->print();
}

std::string X86Or::print() {
    return "or " + op1->print() + ", " + op2->print();
}

std::string X86Xor::print() {
    return "xor " + op1->print() + ", " + op2->print();
}

std::string X86Cmp::print() {
    return "cmp " + op1->print() + ", " + op2->print();
}

std::string X86IMul::print() {
    return "imul " + dest->print() + ", " + op1->print() + ", " + op2->print();
}

std::string X86IDiv::print() {
    return "idiv " + op1->print();
}

std::string X86Jmp::print() {
    std::string ret = "";
    switch (type) {
        case X86Type::Je: ret = "je "; break;
        case X86Type::Jne: ret = "jne "; break;
        case X86Type::Jg: ret = "jg "; break;
        case X86Type::Jl: ret = "jl "; break;
        case X86Type::Jge: ret = "jge "; break;
        case X86Type::Jle: ret = "jle "; break;
        
        default: ret = "jmp ";
    }
    
    ret += op1->print();
    return ret;
}

std::string X86LabelRef::print() {
    return name;
}

std::string X86Imm::print() {
    return std::to_string(value);
}

std::string X86Reg8::print() {
    switch (regType) {
        case X86Reg::AX: return "al";
        case X86Reg::BX: return "bl";
        case X86Reg::CX: return "cl";
        case X86Reg::DX: return "dl";
        
        case X86Reg::SI: return "sil";
        case X86Reg::DI: return "dil";
        case X86Reg::SP: return "spl";
        case X86Reg::BP: return "bpl";
        
        case X86Reg::R8: return "r8b";
        case X86Reg::R9: return "r9b";
        case X86Reg::R10: return "r10b";
        case X86Reg::R11: return "r11b";
        case X86Reg::R12: return "r12b";
        case X86Reg::R13: return "r13b";
        case X86Reg::R14: return "r14b";
        case X86Reg::R15: return "r15b";
    }
    return "";
}

std::string X86Reg16::print() {
    switch (regType) {
        case X86Reg::AX: return "ax";
        case X86Reg::BX: return "bx";
        case X86Reg::CX: return "cx";
        case X86Reg::DX: return "dx";
        
        case X86Reg::SI: return "si";
        case X86Reg::DI: return "di";
        case X86Reg::SP: return "sp";
        case X86Reg::BP: return "bp";
        
        case X86Reg::R8: return "r8w";
        case X86Reg::R9: return "r9w";
        case X86Reg::R10: return "r10w";
        case X86Reg::R11: return "r11w";
        case X86Reg::R12: return "r12w";
        case X86Reg::R13: return "r13w";
        case X86Reg::R14: return "r14w";
        case X86Reg::R15: return "r15w";
    }
    return "";
}

std::string X86Reg32::print() {
    switch (regType) {
        case X86Reg::AX: return "eax";
        case X86Reg::BX: return "ebx";
        case X86Reg::CX: return "ecx";
        case X86Reg::DX: return "edx";
        
        case X86Reg::SI: return "esi";
        case X86Reg::DI: return "edi";
        case X86Reg::SP: return "esp";
        case X86Reg::BP: return "ebp";
        
        case X86Reg::R8: return "r8d";
        case X86Reg::R9: return "r9d";
        case X86Reg::R10: return "r10d";
        case X86Reg::R11: return "r11d";
        case X86Reg::R12: return "r12d";
        case X86Reg::R13: return "r13d";
        case X86Reg::R14: return "r14d";
        case X86Reg::R15: return "r15d";
    }
    return "";
}

std::string X86Reg64::print() {
    switch (regType) {
        case X86Reg::AX: return "rax";
        case X86Reg::BX: return "rbx";
        case X86Reg::CX: return "rcx";
        case X86Reg::DX: return "rdx";
        
        case X86Reg::SI: return "rsi";
        case X86Reg::DI: return "rdi";
        case X86Reg::SP: return "rsp";
        case X86Reg::BP: return "rbp";
        
        case X86Reg::R8: return "r8";
        case X86Reg::R9: return "r9";
        case X86Reg::R10: return "r10";
        case X86Reg::R11: return "r11";
        case X86Reg::R12: return "r12";
        case X86Reg::R13: return "r13";
        case X86Reg::R14: return "r14";
        case X86Reg::R15: return "r15";
    }
    return "";
}

std::string X86RegPtr::print() {
    switch (regType) {
        case X86Reg::AX: return sizeAttr + " [rax]";
        case X86Reg::BX: return sizeAttr + " [rbx]";
        case X86Reg::CX: return sizeAttr + " [rcx]";
        case X86Reg::DX: return sizeAttr + " [rdx]";
        
        case X86Reg::SI: return sizeAttr + " [rsi]";
        case X86Reg::DI: return sizeAttr + " [rdi]";
        case X86Reg::SP: return sizeAttr + " [rsp]";
        case X86Reg::BP: return sizeAttr + " [rbp]";
        
        case X86Reg::R8: return sizeAttr + " [r8]";
        case X86Reg::R9: return sizeAttr + " [r9]";
        case X86Reg::R10: return sizeAttr + " [r10]";
        case X86Reg::R11: return sizeAttr + " [r11]";
        case X86Reg::R12: return sizeAttr + " [r12]";
        case X86Reg::R13: return sizeAttr + " [r13]";
        case X86Reg::R14: return sizeAttr + " [r14]";
        case X86Reg::R15: return sizeAttr + " [r15]";
    }
    return "";
}

std::string X86Mem::print() {
    std::string dest =  sizeAttr + " [";
    dest += base->print();
    dest += offset->print();
    dest += "]";
    return dest;
}

std::string X86String::print() {
    return "OFFSET FLAT:" + value;
}

} // end namespace LLIR
