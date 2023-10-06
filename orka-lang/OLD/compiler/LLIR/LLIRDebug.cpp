#include <iostream>

#include <LLIR/LLIR.hpp>

void LLIRType::dump() {
    switch (type) {
        case LLIRDataType::Void: std::cout << "void"; break;
        case LLIRDataType::I8: std::cout << "i8"; break;
        case LLIRDataType::I16: std::cout << "i16"; break;
        case LLIRDataType::I32: std::cout << "i32"; break;
        case LLIRDataType::I64: std::cout << "i64"; break;
        
        default: {}
    }
}

void LLIRFile::dump() {
    std::cout << ";module: \"" << name << "\"";
    std::cout << std::endl << std::endl;
    
    for (LLIRFunction *func : functions) {
        func->dump();
        std::cout << std::endl;
    }
}

void LLIRFunction::dump() {
    if (entryBlock == nullptr) {
        std::cout << "declare ";
        type->dump();
        std::cout << " " << name;
        
        // TODO: args
        std::cout << "(";
        std::cout << ");" << std::endl;
    } else {
        std::cout << "function ";
        type->dump();
        std::cout << " " << name;
        
        // TODO: args
        std::cout << "(";
        std::cout << ") {" << std::endl;
        
        LLIRBlock *block = entryBlock;
        block->dump();
        while (block->getNext() != nullptr) {
            block = block->getNext();
            block->dump();
        }
        
        std::cout << "}" << std::endl;
    }
}

void LLIRBlock::dump() {
    std::cout << name << ":" << std::endl;
    
    for (LLIRInstruction *instr : code ) {
        std::cout << "  ";
        instr->dump();
        std::cout << std::endl;
    }
}

void LLIRInstruction::dump() {
    switch (opType) {
        case LLIR::Ret: std::cout << "ret "; break;
        case LLIR::RetVoid: std::cout << "ret void "; break;
        
        default: std::cout << "nop ";
    }
    
    if (arg1) arg1->dump();
    std::cout << " ";
    if (arg2) arg2->dump();
    std::cout << " ";
    if (arg3) arg3->dump();
}

void LLIRImm::dump() {
    type->dump();
    std::cout << " " << val;
}

