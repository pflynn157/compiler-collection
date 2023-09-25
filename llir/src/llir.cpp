//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "llir.hpp"

namespace LLIR {

//
// Pointer Type
//
PointerType::PointerType(Type *baseType) {
    this->type = DataType::Ptr;
    this->baseType = baseType;
}

PointerType::PointerType(DataType type) {
    this->type = DataType::Ptr;
    this->baseType = new Type(type);
}

PointerType::~PointerType() {
    if (baseType) delete baseType;
}

//
// Structure Type
//

StructType::StructType(std::string name, std::vector<Type *> elementTypes) {
    this->type = DataType::Struct;
    this->name = name;
    this->elementTypes = elementTypes;
}

StructType::~StructType() {
    for (Type *t : elementTypes) {
        if (t) delete t;
    }
}

std::vector<Type *> StructType::getElementTypes() {
    return elementTypes;
}

//
// Instructions
//

Instruction::Instruction(InstrType type) {
    this->type = type;
    dataType = new Type(DataType::Void);
}

Instruction::~Instruction() {
    if (dataType) delete dataType;
    if (dest) delete dest;
    if (src1) delete src1;
    if (src2) delete src2;
    if (src3) delete src3;
}

void Instruction::setDataType(Type *d) {
    if (dataType) delete dataType;
    dataType = d;
}

void Instruction::setDest(Operand *d) {
    dest = d;
}

void Instruction::setOperand1(Operand *o) {
    src1 = o;
}

void Instruction::setOperand2(Operand *o) {
    src2 = o;
}

void Instruction::setOperand3(Operand *o) {
    src3 = o;
}

InstrType Instruction::getType() {
    return type;
}

Type *Instruction::getDataType() {
    return dataType;
}

Operand *Instruction::getDest() {
    return dest;
}

Operand *Instruction::getOperand1() {
    return src1;
}

Operand *Instruction::getOperand2() {
    return src2;
}

Operand *Instruction::getOperand3() {
    return src3; 
}

//
// Function call instructions
//

FunctionCall::FunctionCall(std::string name, std::vector<Operand *> args) : Instruction(InstrType::Call) {
    this->name = name;
    this->args = args;
}

void FunctionCall::setArgs(std::vector<Operand *> args) {
    this->args = args;
}

std::string FunctionCall::getName() {
    return name;
}

std::vector<Operand *> FunctionCall::getArgs() {
    return args;
}

//
// Blocks
//
Block::Block(std::string name) {
    this->name = name;
}

Block::~Block() {
    for (Instruction *i : instrs) {
        if (i) delete i;
    }
}

void Block::addInstruction(Instruction *i) {
    instrs.push_back(i);
}

void Block::setID(int id) {
    this->id = id;
}

std::string Block::getName() {
    return name;
}

int Block::getID() {
    return id;
}

int Block::getInstrCount() {
    return instrs.size();
}

Instruction *Block::getInstruction(int pos) {
    return instrs.at(pos);
}

//
// Functions
//
Function::Function(std::string name, Linkage linkage) {
    this->name = name;
    this->linkage = linkage;
    dataType = new Type(DataType::Void);
}

Function::~Function() {
    if (dataType) delete dataType;
    for (Block *block : blocks) {
        if (block) delete block;
    }
    for (Reg *reg : varRegs) {
        if (reg) delete reg;
    }
    for (Type *t : args) {
        if (t) delete t;
    }
}

void Function::setDataType(Type *d) {
    if (dataType) delete dataType;
    dataType = d;
}

void Function::setArgs(std::vector<Type *> args) {
    this->args = args;
    for (int i = 0; i<args.size(); i++) {
        Reg *reg = new Reg(std::to_string(i));
        varRegs.push_back(reg);
    }
}

void Function::addArgPair(Type *type, Reg *reg) {
    args.push_back(type);
    varRegs.push_back(reg);
}

void Function::addBlock(Block *block) {
    block->setID(blockID);
    ++blockID;
    blocks.push_back(block);
}

void Function::addBlockAfter(Block *block, Block *newBlock) {
    for (int i = 0; i<blocks.size(); i++) {
        if (block->getID() == blocks.at(i)->getID()) {
            blocks.insert(blocks.begin() + i + 1, newBlock);
            newBlock->setID(blockID);
            ++blockID;
        }
    }
}

std::string Function::getName() {
    return name;
}

Linkage Function::getLinkage() {
    return linkage;
}

int Function::getBlockCount() {
    return blocks.size();
}

Block *Function::getBlock(int pos) {
    return blocks.at(pos);
}

int Function::getArgCount() {
    return args.size(); 
}

Type *Function::getArgType(int pos) {
    return args.at(pos);
}

Reg *Function::getArg(int pos) {
    return varRegs.at(pos);
}

//
// Modules
//
Module::Module(std::string name) {
    this->name = name;
}

Module::~Module() {
    for (Function *f : functions) {
        if (f) delete f;
    }
}

void Module::addFunction(Function *func) {
    functions.push_back(func);
}

void Module::addStringPtr(StringPtr *ptr) {
    strings.push_back(ptr); 
}

std::string Module::getName() {
    return name;
}

int Module::getFunctionCount() {
    return functions.size();
}

Function *Module::getFunction(int pos) {
    return functions.at(pos);
}

int Module::getStringCount() {
    return strings.size();
}

StringPtr *Module::getString(int pos) {
    return strings.at(pos);
}

Function *Module::getFunctionByName(std::string fname) {
    for (Function *f : functions) {
        if (f->getName() == fname) return f;
    }
    return nullptr;
}


} // end namespace LLIR
