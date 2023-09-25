//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>

#include "llir_operand.hpp"

namespace LLIR {

/*! \brief The inner representation of Types
 *
 * This denotes the "type" of Type objects
 */
enum class DataType {
    Void,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
    Ptr,
    Struct
};

enum class Linkage {
    Global,
    Local,
    Extern
};

enum class InstrType {
    None,
    
    // Returns
    Ret,
    RetVoid,
    
    // Integer math
    Add,
    Sub,
    SMul,
    UMul,
    SDiv,
    UDiv,
    SRem,
    URem,
    
    // Bitwise operations
    And,
    Or,
    Xor,
    Not,
    
    // Jumps
    // We're going to use RISC-V style because these will be the easiest
    // to translate on different architectures
    Br,
    Beq,
    Bne,
    Bgt,
    Blt,
    Bge,
    Ble,
    
    // Function calls
    Call,
    
    // Memory
    Alloca,
    StructLoad,
    Load,
    GEP,
    StructStore,
    Store
};

// Forward declarations
class Function;
class Block;
class Instruction;
class Operand;
class Reg;
class StringPtr;

/*! \brief Type- The base of all data types
 *
 * This class represents the basic integer types, and forms the base of more advanced types such
 * as the PointerType and the Structure Type
 */
class Type {
public:
    /*! \brief Create a new type
     *
     * @param type The "type" of our type
     */
    explicit Type(DataType type) {
        this->type = type;
    }
    
    virtual ~Type() {}
    
    // Convience creators for types
    static Type *createVoidType() { return new Type(DataType::Void); }
    static Type *createI8Type() { return new Type(DataType::I8); }
    static Type *createI16Type() { return new Type(DataType::I16); }
    static Type *createI32Type() { return new Type(DataType::I32); }
    static Type *createI64Type() { return new Type(DataType::I64); }
    
    /*! \brief Returns what kind of type you have
     *
     */
    DataType getType() { return type; }
    
    virtual void print();
protected:
    explicit Type() {}
    DataType type = DataType::Void;
};

/*! \brief Represents a pointer
 *
 * This class is used to represents pointers. This class can be used recursively with other Types or
 * other Pointer Types.
 */
class PointerType : public Type {
public:
    /*! \brief Create a new Pointer Type
     *
     * Creates a new pointer type with another Type (or PointerType) object as the base.
     */
    explicit PointerType(Type *baseType);
    
    /*! \brief Create a new Pointer Type
     *
     * Creates a new pointer type based on a value from the type enumeration
     */
    explicit PointerType(DataType type);
    
    ~PointerType();
    
    static PointerType *createVoidPtrType() { return new PointerType(DataType::Void); }
    static PointerType *createI8PtrType() { return new PointerType(DataType::I8); }
    static PointerType *createI16PtrType() { return new PointerType(DataType::I16); }
    static PointerType *createI32PtrType() { return new PointerType(DataType::I32); }
    static PointerType *createI64PtrType() { return new PointerType(DataType::I64); }
    
    /*! \brief Returns the base type of the pointer
     *
     */
    Type *getBaseType() { return baseType; }
    
    void print();
private:
    Type *baseType = nullptr;
};

/*! \brief Represents a structure
 *
 * This class is used to represent structured type. Members of a structure type are
 * represented by an internal array of types.
 */
class StructType : public Type {
public:
    /*! \brief Create a new structure type
     *
     * @param name The name of the structure
     * @param elementTypes The types of each member
     */
    explicit StructType(std::string name, std::vector<Type *> elementTypes);
    ~StructType();
    
    /*! \brief Returns a list of the element types
     */
    std::vector<Type *> getElementTypes();
    
    void print();
private:
    std::string name = "";
    std::vector<Type *> elementTypes;
};

/*! \brief Represents an instruction in LLIR
 *
 * This class is used to represent a structure in LLIR. Although inheritence for more specific instructions can
 * be done, in general using this class should suffice for most instructions. If you are working in LLIR, only
 * extend this class if you have a specific need.
 */
class Instruction {
public:
    /*! \brief Create a new instruction
     *
     * @param type The type of instruction to be created
     */
    explicit Instruction(InstrType type);
    ~Instruction();
    
    /*! \brief Set the data type of the instruction
     *
     * Sets the data type for the instruction. This is assumed to be Void by default, but obviously this
     * will not work on math and return types. You must set this or you could run into problems.
     *
     * @param d The type of the instruction
     */
    void setDataType(Type *d);
    
    /*! \brief Sets the destination for the instruction
     *
     * Note this is not needed for all instructions
     *
     * @param d The operand representing the destination
     */
    void setDest(Operand *d);
    
    /*! \brief Sets the first source operand for the instruction
     *
     * @param o The operand
     */
    void setOperand1(Operand *o);
    
    /*! \brief Sets the second source operand for the instruction
     *
     * @param o The operand
     */
    void setOperand2(Operand *o);
    
    /*! \brief Sets the third source operand for the instruction
     *
     * @param o The operand
     */
    void setOperand3(Operand *o);
    
    /*! \brief Returns the instruction type
     *
     */
    InstrType getType();
    
    /*! \brief Returns the data type of the instruction
     *
     */
    Type *getDataType();
    
    /*! \brief Returns the instruction's destination
     *
     */
    Operand *getDest();
    
    /*! \brief Returns the first source operand
     *
     */
    Operand *getOperand1();
    
    /*! \brief Returns the second source operand
     *
     */
    Operand *getOperand2();
    
    /*! \brief Returns the third source operand
     *
     */
    Operand *getOperand3();
    
    virtual void print();
protected:
    Type *dataType;
    InstrType type = InstrType::None;
    Operand *dest = nullptr;
    Operand *src1 = nullptr;
    Operand *src2 = nullptr;
    Operand *src3 = nullptr;
};

/*! \brief Represents a function call instruction
 *
 * An extended form of Instruction used to represent function calls
 */
class FunctionCall : public Instruction {
public:
    /*! \brief Creates a new function call instruction
     *
     * @param name The name of the function
     * @param args The function arguments to be passed
     */
    explicit FunctionCall(std::string name, std::vector<Operand *> args);
    
    /*! \brief Sets the function arguments
     *
     * @param args The function arguments to be passed
     */
    void setArgs(std::vector<Operand *> args);
    
    /*! \brief Returns the name of the function
     *
     */
    std::string getName();
    
    /*! \brief Returns the arguments of the function
     *
     */
    std::vector<Operand *> getArgs();
    
    void print();
private:
    std::string name = "";
    std::vector<Operand *> args;
};

/*! \brief Represents a basic block in LLIR
 *
 * Basic blocks form the base of instructions in LLIR. A basic block contains a variable
 * number of instructions, and terminates with either a branch or a return.
 */
class Block {
public:
    /*! \brief Creates a new LLIR block
     *
     * You should use this constructor directly when you need to create a basic block, but
     * when you do not want it to become the current insert point and/or you wish to insert
     * it somewhere else within the function
     *
     * @param name The name of the block. This must be unique.
     */
    explicit Block(std::string name);
    ~Block();
    
    /*! \brief Adds an instruction to the block
     *
     * @param i The instruction to add
     */
    void addInstruction(Instruction *i);
    
    /*! \brief Sets the unique ID for the block
     *
     * IMPORTANT: You should not use this function directly. It is managed by the parent function.
     *
     * @param id The ID to set.
     */
    void setID(int id);
    
    /*! \brief Returns the name of the block
     *
     */
    std::string getName();
    
    /*! \brief Returns the ID of the block
     *
     */
    int getID();
    
    /*! \brief Returns the number of instructions in the block
     *
     */
    int getInstrCount();
    
    /*! \brief Returns the instruction at a given position
     *
     */
    Instruction *getInstruction(int pos);
    
    void print();
private:
    std::string name = "";
    std::vector<Instruction *> instrs;
    int id = 0;
};

/*! \brief Represents a function in LLIR
 *
 * Represents a function in LLIR. An LLIR function can be a complete function with a body,
 * or it can be an extern function for linking purposes.
 */
class Function {
public:
    /*! \brief Creates a new function
     *
     * NOTE: In most cases, you should use the static version of this function.
     *
     * @param name The name of the function
     * @param linkage The linkage type of the function
     */
    explicit Function(std::string name, Linkage linkage);
    ~Function();
    
    /*! \brief Creates and returns a new function
     *
     * @param name The name of the function
     * @param linkage The linkage type of the function
     * @param dataType The data type (return type) of the function
     */
    static Function *Create(std::string name, Linkage linkage, Type *dataType) {
        Function *func = new Function(name, linkage);
        func->setDataType(dataType);
        return func;
    }
    
    /*! \brief Sets the data type (return type) of the function
     *
     * @param d The type to set
     */
    void setDataType(Type *d);
    
    Type *getDataType() {
        return dataType;
    }
    
    /*! \brief Sets the function arguments of a function
     *
     * @param args The function arguments to set
     */
    void setArgs(std::vector<Type *> args);
    
    /*! \brief Sets an type-register pair for the function
     *
     * This adds an argument of a specific type with an associated register to the function. Note
     * that in most cases, you should not have to use this.
     *
     * @param type The type of the argument
     * @param reg The associated register for the argument
     */
    void addArgPair(Type *type, Reg *reg);
    
    /*! \brief Adds a block to the function
     *
     * @param block THe block to add
     */
    void addBlock(Block *block);
    
    /*! \brief Adds a new block after an existing block
     *
     * @param block The current block- the "before" block
     * @param newBlock The new block to add
     */
    void addBlockAfter(Block *block, Block *newBlock);
    
    /*! \brief Returns the name of the function
     *
     */
    std::string getName();
    
    /*! \brief Returns the linkage of the function
     *
     */
    Linkage getLinkage();
    
    /*! \brief Returns the number of blocks in the function
     *
     */
    int getBlockCount();
    
    /*! \brief Returns a block at given position
     *
     */
    Block *getBlock(int pos);
    
    /*! \brief Returns the number of arguments for the function
     *
     */
    int getArgCount();
    
    /*! \brief Returns the argument type at a given position
     *
     */
    Type *getArgType(int pos);
    
    /*! \brief Returns the associated register for an argument at a given position
     *
     */
    Reg *getArg(int pos);
    
    void print();
private:
    Type *dataType;
    std::string name = "";
    Linkage linkage = Linkage::Local;
    std::vector<Block *> blocks;
    std::vector<Type *> args;
    std::vector<Reg *> varRegs;
    int blockID = 1;
};

/*! \brief Represents a module (compilation unit) in LLIR
 *
 * A module is the base element of an LLIR project. A module corresponds to a single compilation unit.
 */
class Module {
public:
    /*! \brief Creates a new module
     *
     * @param name The name of the module.
     */
    explicit Module(std::string name);
    ~Module();
    
    /*! \brief Adds a function to the module
     *
     */
    void addFunction(Function *func);
    
    /*! \brief Adds a global string constant to the module
     *
     * In most cases, this would correspond the .data section on Linux.
     *
     * @param ptr The pointer to add
     */
    void addStringPtr(StringPtr *ptr);
    
    /*! \brief Returns the name of the module
     *
     */
    std::string getName();
    
    /*! \brief Returns the number of functions in the module
     *
     */
    int getFunctionCount();
    
    /*! \brief Returns a function at a given position in the module
     *
     */
    Function *getFunction(int pos);
    
    /*! \brief Returns the number of string constants in the module
     *
     */
    int getStringCount();
    
    /*! \brief Returns a string pointer at a given position
     *
     */
    StringPtr *getString(int pos);
    
    /*! \brief Searches for and returns a function based on a given name
     *
     */
    Function *getFunctionByName(std::string fname);
    
    /*! \brief Hardware transformation
     *
     * This runs the transform layer. The transform layer is in charge of converting virtual registers
     * to real-world hardware elements. This MUST be called before using a hardware translation
     * layer.
     */
    void transform();
    
    void print();
private:
    std::string name = "";
    std::vector<Function *> functions;
    std::vector<StringPtr *> strings;
};

} // end namespace LLIR

