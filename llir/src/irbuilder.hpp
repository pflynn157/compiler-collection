//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include "llir.hpp"

namespace LLIR {

/*! \brief IRBuilder
 *
 * This class is meant to make it easy to build an LLIR module. In general, you should use this class when creating
 * LLIR unless you are experimenting or doing something internal to LLIR. This class will take care of many tedious
 * tasks such as register naming, operands, and so forth.
 */
class IRBuilder {
public:

    /*! \brief Creates an IRBuilder object
     *
     * The IRBuilder is used to easily construct an LLIR module.
     *
     * @param mod The module we are going to build
     */
    explicit IRBuilder(Module *mod);

    /*! \brief Set the current function
     *
     * Sets the current function we are adding too. This must be called at least once, or
     * the program could create a segmentation fault
     *
     * @param func The function we are setting
     */
    void setCurrentFunction(Function *func) {
        currentFunc = func;
        regCounter = currentFunc->getArgCount();
    };
    
    Function *getCurrentFunction() {
        return currentFunc;
    }
    
    /*! \brief Create a new block and set the insert point
     *
     * This creates a new block, which holds individual lines of code within a function. This function will
     * also set the IRBuilder to add new lines of code to this block (the insert point). To create a block without
     * setting the insert point, use the regular constructor.
     *
     * @param name The name of the block
     */
    Block *createBlock(std::string name);
    
    /*! \brief Adds a block to the current function
     */
    void addBlock(Block *block);
    
    /*! \brief Adds a block to the current function after an existing block
     *
     * @param block The original block
     * @param newBlock The new block to be added
     */
    void addBlockAfter(Block *block, Block *newBlock);
    
    /*! \brief Sets the insert point
     *
     * Sets the insert point for future instructions to a certain block
     */
    void setInsertPoint(Block *block);
    
    /*! \brief Returns the current insert point
     */
    Block *getInsertPoint();
    
    //
    // Operand builders
    //
    
    /*! \brief Creates an 8-bit (1 byte) integer value
     */
    Operand *createI8(int8_t val);
    
    /*! \brief Creates a 16-bit (2 byte) integer value
     */
    Operand *createI16(int16_t val);
    
    /*! \brief Creates a 32-bit (4 byte) integer value
     */
    Operand *createI32(int val);
    
    /*! \brief Creates a 64-bit (8 byte) integer value
     */
    Operand *createI64(int64_t val);
    
    /*! \brief Creates a string constant
     */
    Operand *createString(std::string val);
    
    //
    // Instruction builders
    //
    
    /*! \brief Allocates space on the stack for a variable
     */
    Reg *createAlloca(Type *type);
    
    /*! \brief Creates a store instruction (storing a value to memory)
     */
    Instruction *createStore(Type *type, Operand *op, Operand *dest);
    
    /*! \brief Creates a store instruction for stack-based structures
     */
    Instruction *createStructStore(Type *type, Operand *ptr, int index, Operand *val);
    
    /*! \brief Creates a load instruction for memory locations
     */
    Reg *createLoad(Type *type, Operand *src);
    
    /*! \brief Creates a load for stack-based structures
     */
    Reg *createStructLoad(Type *type, Operand *src, int index);
    
    /*! \brief Returns a pointer to a location within a block of memory (generally, an array)
     */
    Operand *createGEP(Type *type, Operand *ptr, Operand *index);
    
    /*! \brief Creates an add instruction
     */
    Operand *createAdd(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a subtraction instruction
     */
    Operand *createSub(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a signed multiplication instruction
     */
    Operand *createSMul(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a signed division instruction
     */
    Operand *createSDiv(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a bitwise AND instruction
     */
    Operand *createAnd(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a bitwise OR instruction
     */
    Operand *createOr(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a bitwise XOR instruction
     */
    Operand *createXor(Type *type, Operand *op1, Operand *op2);
    
    /*! \brief Creates a negation instruction
     */
    Operand *createNeg(Type *type, Operand *op1);
    
    /*! \brief Creates a branch-if-equal comparison/jump instruction
     */
    Operand *createBeq(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates a branch-if-not-equal comparison/jump instruction
     */
    Operand *createBne(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates a branch-if-greater-than comparison/jump instruction
     */
    Operand *createBgt(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates a branch-if-less-than comparison/jump instruction
     */
    Operand *createBlt(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates a branch-if-greater-than-or-equal comparison/jump instruction
     */
    Operand *createBge(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates a branch-if-less-than-or-equal comparison/jump instruction
     */
    Operand *createBle(Type *type, Operand *op1, Operand *op2, Block *destBlock);
    
    /*! \brief Creates an unconditional branch instruction
     */
    Instruction *createBr(Block *block);
    
    /*! \brief Creates a function call that does not return any value
     */
    Instruction *createVoidCall(std::string name, std::vector<Operand *> args);
    
    /*! \brief Creates a function call that does return a value
     */
    Reg *createCall(Type *type, std::string name, std::vector<Operand *> args);
    
    /*! \brief Creates a function return (returns void)
     */
    Instruction *createRetVoid();
    
    /*! \brief Creates a function return (returns a value)
     */
    Instruction *createRet(Type *type, Operand *op);
    
    /*! \brief Adds an instruction to the current insert block
     */
    void addInstruction(Instruction *instr) {
        currentBlock->addInstruction(instr);
    }
protected:
    Operand *createBinaryOp(Type *type, Operand *op1, Operand *op2, InstrType iType, Block *destBlock = nullptr);
private:
    Module *mod;
    Function *currentFunc;
    Block *currentBlock;
    int regCounter = 0;
    int lblCounter = 0;
};

}
