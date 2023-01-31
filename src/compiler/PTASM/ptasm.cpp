#include <iostream>

#include "ptasm.hpp"

//
// Represents an assembly data type
//
AsmType::AsmType(V_AsmType type) {
    this->type = type;
}

void AsmType::print() {
    switch (type) {
        case V_AsmType::Void: std::cout << "void"; break;
        case V_AsmType::I8: std::cout << "i8"; break;
        case V_AsmType::I16: std::cout << "i16"; break;
        case V_AsmType::I32: std::cout << "i32"; break;
        case V_AsmType::I64: std::cout << "i64"; break;
        case V_AsmType::F32: std::cout << "f32"; break;
        case V_AsmType::F64: std::cout << "f64"; break;
        case V_AsmType::Array: break;
        case V_AsmType::Struct: break;
    
        default: std::cout << "NULL";
    }
}

//
// Represents an assembly operand
//
AsmOperand::AsmOperand(V_AsmType type) {
    this->type = type;
}

V_AsmType AsmOperand::getType() {
    return type;
}

void AsmOperand::print() {
    switch (type) {
        case V_AsmType::RetReg: std::cout << "ret.reg"; break;
        
        default: std::cout << "NULL";
    }
}

// Represents an integer operand
AsmInt::AsmInt(int value) : AsmOperand(V_AsmType::Int) {
    this->value = value;
}

int AsmInt::getValue() {
    return value;
}

void AsmInt::print() {
    std::cout << "#" << value;
}

// Represents an ID operand
AsmId::AsmId(std::string value) : AsmOperand(V_AsmType::Id) {
    this->value = value;
}

std::string AsmId::getValue() {
    return value;
}

void AsmId::print() {
    std::cout << "%" << value;
}

//
// Represents an assembly instruction
//
AsmInstruction::AsmInstruction(V_AsmType type) {
    this->type = type;
}

void AsmInstruction::setDataType(AsmType *dataType) {
    this->dataType = dataType;
}

void AsmInstruction::setDestOperand(AsmOperand *dest) {
    this->dest = dest;
}

void AsmInstruction::setSrc1Operand(AsmOperand *src1) {
    this->src1 = src1;
}

void AsmInstruction::setSrc2Operand(AsmOperand *src2) {
    this->src2 = src2;
}

V_AsmType AsmInstruction::getType() {
    return type;
}

AsmType *AsmInstruction::getDataType() {
    return dataType;
}

AsmOperand *AsmInstruction::getDestOperand() {
    return dest;
}

AsmOperand *AsmInstruction::getSrc1Operand() {
    return src1;
}

AsmOperand *AsmInstruction::getSrc2Operand() {
    return src2;
}

void AsmInstruction::print() {
    switch (type) {
        // Math
        case V_AsmType::Add: std::cout << "add"; break;
        case V_AsmType::Sub: std::cout << "sub"; break;
        case V_AsmType::Mul: std::cout << "mul"; break;
        case V_AsmType::UDiv: std::cout << "udiv"; break;
        case V_AsmType::SDiv: std::cout << "sdiv"; break;
        case V_AsmType::URem: std::cout << "urem"; break;
        case V_AsmType::SRem: std::cout << "srem"; break;
        
        // Load and store
        case V_AsmType::Alloca: std::cout << "alloc"; break;
        case V_AsmType::Ld: std::cout << "ld"; break;
        case V_AsmType::St: std::cout << "st"; break;
        
        // Branch
        case V_AsmType::Br: std::cout << "br"; break;
        case V_AsmType::Beq: std::cout << "beq"; break;
        case V_AsmType::Bne: std::cout << "bne"; break;
        case V_AsmType::Bg: std::cout << "bg"; break;
        case V_AsmType::Bl: std::cout << "bl"; break;
        case V_AsmType::Bge: std::cout << "bge"; break;
        case V_AsmType::Ble: std::cout << "ble"; break;
        
        // Control flow
        case V_AsmType::Call: std::cout << "call"; break;
        case V_AsmType::Ret: std::cout << "ret"; break;
        
        // NOP
        default: std::cout << "nop";
    }
    
    // Print data type
    std::cout << " ";
    dataType->print();
    std::cout << " ";
    
    // Print arguments if the exist
    if (dest) {
        std::cout << " ";
        dest->print();
    }
    
    if (src1) {
        std::cout << ", ";
        src1->print();
    }
    
    if (src2) {
        std::cout << ", ";
        src2->print();
    }
    
    std::cout << std::endl;
}

//
// Represents a block of assembly functions
//
AsmBlock::AsmBlock(std::string name) {
    this->name = name;
}

void AsmBlock::addInstruction(AsmInstruction *instr) {
    block.push_back(instr);
}

std::string AsmBlock::getName() {
    return name;
}

size_t AsmBlock::getInstrCount() {
    return block.size();
}

AsmInstruction *AsmBlock::getInstrAt(size_t i) {
    return block.at(i);
}

void AsmBlock::print() {
    std::cout << name << ":" << std::endl;
    for (AsmInstruction *i : block) {
        std::cout << "\t";
        i->print();
    }
}

//
// Represents a portable assembly function
//
AsmFunction::AsmFunction(std::string name) {
    this->name = name;
}

void AsmFunction::addBlock(AsmBlock *block) {
    blocks.push_back(block);
}

std::string AsmFunction::getName() {
    return name;
}

size_t AsmFunction::getBlockCount() {
    return blocks.size();
}

AsmBlock *AsmFunction::getBlockAt(size_t i) {
    return blocks.at(i);
}

void AsmFunction::print() {
    std::cout << "func " << name << ":" << std::endl;
    for (AsmBlock *block : blocks) {
        block->print();
    }
}

//
// Represents a portable assembly file
//
AsmFile::AsmFile(std::string name) {
    this->name = name;
}

void AsmFile::addFunction(AsmFunction *func) {
    functions.push_back(func);
}

std::string AsmFile::getName() {
    return name;
}

size_t AsmFile::getFunctionCount() {
    return functions.size();
}

AsmFunction *AsmFile::getFunctionAt(size_t pos) {
    return functions.at(pos);
}

void AsmFile::print() {
    std::cout << std::endl;
    std::cout << ";FILE: " << name << std::endl;
    std::cout << std::endl;
    
    for (AsmFunction *func : functions) {
        func->print();
        std::cout << std::endl;
    }
}


