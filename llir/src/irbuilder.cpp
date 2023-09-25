//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <llir.hpp>
#include <irbuilder.hpp>

namespace LLIR {

IRBuilder::IRBuilder(Module *mod) {
    this->mod = mod;
}

Block *IRBuilder::createBlock(std::string name) {
    currentBlock = new Block(name);
    currentFunc->addBlock(currentBlock);
    return currentBlock;
}

void IRBuilder::addBlock(Block *block) {
    currentFunc->addBlock(block);
}

void IRBuilder::addBlockAfter(Block *block, Block *newBlock) {
    currentFunc->addBlockAfter(block, newBlock);
}

void IRBuilder::setInsertPoint(Block *block) {
    currentBlock = block;
}

Block *IRBuilder::getInsertPoint() {
    return currentBlock;
}

Operand *IRBuilder::createI8(int8_t val) {
    return new Imm(val);
}

Operand *IRBuilder::createI16(int16_t val) {
    return new Imm(val);
}

Operand *IRBuilder::createI32(int val) {
    return new Imm(val);
}

Operand *IRBuilder::createI64(int64_t val) {
    return new Imm(val);
}

Operand *IRBuilder::createString(std::string val) {
    std::string name = "STR" + std::to_string(lblCounter);
    ++lblCounter;
    
    std::string val2 = "";
    for (char c : val) {
        if (c == '\n') val2 += "\\n";
        else val2 += c;
    }
    
    StringPtr *ptr = new StringPtr(name, val2);
    mod->addStringPtr(ptr);
    
    return ptr;
}

Reg *IRBuilder::createAlloca(Type *type) {
    Instruction *alloc = new Instruction(InstrType::Alloca);
    alloc->setDataType(type);
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    alloc->setDest(dest);
    
    currentBlock->addInstruction(alloc);
    return dest;
}

Instruction *IRBuilder::createStore(Type *type, Operand *op, Operand *dest) {
    Instruction *store = new Instruction(InstrType::Store);
    store->setDataType(type);
    store->setOperand1(op);
    store->setOperand2(dest);
    currentBlock->addInstruction(store);
    return store;
}

Instruction *IRBuilder::createStructStore(Type *type, Operand *ptr, int index, Operand *val) {
    Instruction *op = new Instruction(InstrType::StructStore);
    op->setDataType(type);
    op->setOperand1(ptr);
    op->setOperand2(new Imm(index));
    op->setOperand3(val);
    
    currentBlock->addInstruction(op);
    return op;
}

Reg *IRBuilder::createLoad(Type *type, Operand *src) {
    Instruction *load = new Instruction(InstrType::Load);
    load->setDataType(type);
    load->setOperand1(src);
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    load->setDest(dest);
    
    currentBlock->addInstruction(load);
    return dest;
}

Reg *IRBuilder::createStructLoad(Type *type, Operand *src, int index) {
    Instruction *load = new Instruction(InstrType::StructLoad);
    load->setDataType(type);
    load->setOperand1(src);
    load->setOperand2(new Imm(index));
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    load->setDest(dest);
    
    currentBlock->addInstruction(load);
    return dest;
}

Operand *IRBuilder::createBinaryOp(Type *type, Operand *op1, Operand *op2, InstrType iType, Block *destBlock) {
    if (op1->getType() == OpType::Imm && op2->getType() == OpType::Imm) {
        Imm *imm1 = static_cast<Imm *>(op1);
        Imm *imm2 = static_cast<Imm *>(op2);
        
        switch (iType) {
            case InstrType::Add: return new Imm(imm1->getValue() + imm2->getValue());
            case InstrType::Sub: return new Imm(imm1->getValue() - imm2->getValue());
            case InstrType::SMul: return new Imm(imm1->getValue() * imm2->getValue());
            case InstrType::SDiv: return new Imm(imm1->getValue() / imm2->getValue());
            case InstrType::And: return new Imm(imm1->getValue() & imm2->getValue());
            case InstrType::Or: return new Imm(imm1->getValue() | imm2->getValue());
            case InstrType::Xor: return new Imm(imm1->getValue() ^ imm2->getValue());
            
            default: {}
        }
    }
    
    Instruction *op = new Instruction(iType);
    op->setDataType(type);
    op->setOperand1(op1);
    op->setOperand2(op2);
    
    if (destBlock != nullptr) op->setOperand3(new Label(destBlock->getName()));
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    op->setDest(dest);
    
    currentBlock->addInstruction(op);
    return dest;
}

Operand *IRBuilder::createGEP(Type *type, Operand *ptr, Operand *index) {
    return createBinaryOp(type, ptr, index, InstrType::GEP);
}

Operand *IRBuilder::createAdd(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::Add);
}

Operand *IRBuilder::createSub(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::Sub);
}

Operand *IRBuilder::createSMul(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::SMul);
}

Operand *IRBuilder::createSDiv(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::SDiv);
}

Operand *IRBuilder::createAnd(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::And);
}

Operand *IRBuilder::createOr(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::Or);
}

Operand *IRBuilder::createXor(Type *type, Operand *op1, Operand *op2) {
    return createBinaryOp(type, op1, op2, InstrType::Xor);
}

Operand *IRBuilder::createNeg(Type *type, Operand *op1) {
    if (op1->getType() == OpType::Imm) {
        Imm *imm = static_cast<Imm *>(op1);
        int val = imm->getValue() * -1;
        imm->setValue(val);
        return imm;    
    }
    
    Instruction *op = new Instruction(InstrType::Not);
    op->setDataType(type);
    op->setOperand1(op1);
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    op->setDest(dest);
    
    currentBlock->addInstruction(op);
    return dest;
}

Operand *IRBuilder::createBeq(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    if (op1->getType() == OpType::Imm && op2->getType() == OpType::Imm) {
        Imm *imm1 = static_cast<Imm *>(op1);
        Imm *imm2 = static_cast<Imm *>(op2);
        if (imm1->getValue() == imm2->getValue()) {
            Label *lbl = new Label(destBlock->getName());
            Instruction *op = new Instruction(InstrType::Br);
            op->setOperand1(lbl);
            
            Reg *dest = new Reg(std::to_string(regCounter));
            ++regCounter;
            op->setDest(dest);
            
            currentBlock->addInstruction(op);
            return dest;
        }
    }
    return createBinaryOp(type, op1, op2, InstrType::Beq, destBlock);
}

Operand *IRBuilder::createBne(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    return createBinaryOp(type, op1, op2, InstrType::Bne, destBlock);
}

Operand *IRBuilder::createBgt(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    return createBinaryOp(type, op1, op2, InstrType::Bgt, destBlock);
}

Operand *IRBuilder::createBlt(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    return createBinaryOp(type, op1, op2, InstrType::Blt, destBlock);
}

Operand *IRBuilder::createBge(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    return createBinaryOp(type, op1, op2, InstrType::Bge, destBlock);
}

Operand *IRBuilder::createBle(Type *type, Operand *op1, Operand *op2, Block *destBlock) {
    return createBinaryOp(type, op1, op2, InstrType::Ble, destBlock);
}

Instruction *IRBuilder::createBr(Block *block) {
    Label *lbl = new Label(block->getName());
    Instruction *op = new Instruction(InstrType::Br);
    op->setOperand1(lbl);
    currentBlock->addInstruction(op);
    return op;
}

Instruction *IRBuilder::createVoidCall(std::string name, std::vector<Operand *> args) {
    FunctionCall *fc = new FunctionCall(name, args);
    currentBlock->addInstruction(fc);
    return fc;
}

Reg *IRBuilder::createCall(Type *type, std::string name, std::vector<Operand *> args) {
    FunctionCall *fc = new FunctionCall(name, args);
    fc->setDataType(type);
    
    Reg *dest = new Reg(std::to_string(regCounter));
    ++regCounter;
    fc->setDest(dest);
    
    currentBlock->addInstruction(fc);
    return dest;
}

Instruction *IRBuilder::createRetVoid() {
    Instruction *ret = new Instruction(InstrType::Ret);
    ret->setDataType(Type::createVoidType());
    currentBlock->addInstruction(ret);
    return ret;
}

Instruction *IRBuilder::createRet(Type *type, Operand *op) {
    Instruction *ret = new Instruction(InstrType::Ret);
    ret->setDataType(type);
    ret->setOperand1(op);
    currentBlock->addInstruction(ret);
    return ret;
}

} // end namespace LLIR
