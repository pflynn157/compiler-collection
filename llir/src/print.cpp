//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <llir.hpp>

namespace LLIR {

void Module::print() {
    std::cout << "#module " << name << std::endl;
    std::cout << std::endl;
    for (Function *func : functions) func->print();
}

void Type::print() {
    switch (type) {
        case DataType::Void: std::cout << "void"; break;
        case DataType::I8: std::cout << "i8"; break;
        case DataType::I16: std::cout << "i16"; break;
        case DataType::I32: std::cout << "i32"; break;
        case DataType::I64: std::cout << "i64"; break;
        case DataType::F32: std::cout << "f32"; break;
        case DataType::F64: std::cout << "f64"; break;
        case DataType::Ptr: std::cout << "ptr"; break;
    }
}

void PointerType::print() {
    std::cout << "*";
    baseType->print();
}

void StructType::print() {
    std::cout << name << "{";
    for (int i = 0; i<elementTypes.size(); i++) {
        elementTypes.at(i)->print();
        if (i + 1 < elementTypes.size()) std::cout << ", ";
    }
    std::cout << "}";
}

void Function::print() {
    switch (linkage) {
        case Linkage::Local: std::cout << "local "; break;
        case Linkage::Global: std::cout << "global "; break;
        case Linkage::Extern: std::cout << "extern "; break;
    }
    dataType->print();
    std::cout << " ";
    std::cout << name;
    
    std::cout << "(";
    for (int i = 0; i<args.size(); i++) {
        varRegs.at(i)->print();
        std::cout << ":";
        args.at(i)->print();
        if (i + 1 < args.size()) std::cout << ", ";
    }
    std::cout << ")";
    
    if (blocks.size() == 0) {
        std::cout << ";";
    } else {
        std::cout << " {" << std::endl;
        for (Block *block : blocks) block->print();
        std::cout << "}";
    }
    std::cout << std::endl;
}

void Block::print() {
    std::cout << name << ":" << std::endl;
    for (Instruction *i : instrs) {
        std::cout << "  ";
        i->print();
    }
}

void Instruction::print() {
    if (dest) {
        dest->print();
        std::cout << " = ";
    }

    switch (type) {
        case InstrType::None: std::cout << "?? "; break;
        
        case InstrType::Ret: std::cout << "ret "; break;
        case InstrType::RetVoid: std::cout << "retvoid "; break;
        
        case InstrType::Add: std::cout << "add "; break;
        case InstrType::Sub: std::cout << "sub "; break;
        case InstrType::UMul: std::cout << "umul "; break;
        case InstrType::SMul: std::cout << "smul "; break;
        case InstrType::UDiv: std::cout << "udiv "; break;
        case InstrType::SDiv: std::cout << "sdiv "; break;
        case InstrType::URem: std::cout << "urem "; break;
        case InstrType::SRem: std::cout << "srem "; break;
        
        case InstrType::And: std::cout << "and "; break;
        case InstrType::Or: std::cout << "or "; break;
        case InstrType::Xor: std::cout << "xor "; break;
        case InstrType::Not: std::cout << "not "; break;
        
        case InstrType::Br: std::cout << "br "; break;
        case InstrType::Beq: std::cout << "beq "; break;
        case InstrType::Bne: std::cout << "bne "; break;
        case InstrType::Bgt: std::cout << "bgt "; break;
        case InstrType::Blt: std::cout << "blt "; break;
        case InstrType::Bge: std::cout << "bge "; break;
        case InstrType::Ble: std::cout << "ble "; break;
        
        case InstrType::Alloca: std::cout << "alloca "; break;
        case InstrType::StructLoad: std::cout << "load.struct "; break;
        case InstrType::Load: std::cout << "load "; break;
        case InstrType::GEP: std::cout << "getelementptr "; break;
        case InstrType::StructStore: std::cout << "store.struct "; break;
        case InstrType::Store: std::cout << "store "; break;
    }
    dataType->print();
    std::cout << " ";
    
    if (src1) {
        src1->print();
    }
    if (src2) {
        std::cout << ", ";
        src2->print();
    }
    if (src3) {
        std::cout << ", ";
        src3->print();
    }
    std::cout << ";" << std::endl;
}

void FunctionCall::print() {
    if (dest) {
        dest->print();
        std::cout << " = ";
    }
    std::cout << "call ";
    if (dataType) {
        dataType->print();
        std::cout << " ";
    }
    
    std::cout << name << "(";
    for (int i = 0; i<args.size(); i++) {
        args.at(i)->print();
        if (i + 1 < args.size()) {
            std::cout << ", ";
        }
    }
    std::cout << ");" << std::endl;
}

void Imm::print() {
    std::cout << imm;
}

void Reg::print() {
    std::cout << "%" << name;
}

void Label::print() {
    std::cout << name;
}

void StringPtr::print() {
    std::cout << "$" << name << "(\"" << val << "\")";
}

void Mem::print() {
    std::cout << "[" << name << "]";
}

void HReg::print() {
    std::cout << "r" << num;
}

void AReg::print() {
    std::cout << "a" << num;
}

void PReg::print() {
    std::cout << "p" << num;
}

} // end namespace LLIR
