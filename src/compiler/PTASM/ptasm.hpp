#pragma once

#include <string>
#include <vector>

enum class V_AsmType {
    Nop,
    
    // Math
    Add, Sub, Mul, UDiv, SDiv, URem, SRem,
    
    // Load and store
    Alloca,
    Ld, St,
    
    // Branch
    Br, Beq, Bne, Bg, Bl, Bge, Ble,
    
    // Control flow
    Call, Ret,
    
    // Operands
    Id,
    Reg, RetReg, ArgReg,
    Int,
    
    // Types
    Void, I8, I16, I32, I64, F32, F64,
    Array,
    Struct
};

class AsmType {
public:
    explicit AsmType(V_AsmType type);
    void print();
protected:
    V_AsmType type = V_AsmType::Void;
};

class AsmOperand {
public:
    explicit AsmOperand(V_AsmType type);
    V_AsmType getType();
    virtual void print();
protected:
    V_AsmType type = V_AsmType::Nop;
};

class AsmInt : public AsmOperand {
public:
    explicit AsmInt(int value);
    int getValue();
    void print();
protected:
    int value = 0;
};

class AsmId : public AsmOperand {
public:
    explicit AsmId(std::string value);
    std::string getValue();
    void print();
protected:
    std::string value = "";
};

class AsmInstruction {
public:
    explicit AsmInstruction(V_AsmType type);
    void setDataType(AsmType *dataType);
    void setDestOperand(AsmOperand *dest);
    void setSrc1Operand(AsmOperand *src1);
    void setSrc2Operand(AsmOperand *src2);
    V_AsmType getType();
    AsmType *getDataType();
    AsmOperand *getDestOperand();
    AsmOperand *getSrc1Operand();
    AsmOperand *getSrc2Operand();
    void print();
protected:
    V_AsmType type = V_AsmType::Nop;
    AsmType *dataType = nullptr;
    AsmOperand *dest = nullptr;
    AsmOperand *src1 = nullptr;
    AsmOperand *src2 = nullptr;
};

class AsmBlock {
public:
    explicit AsmBlock(std::string name);
    void addInstruction(AsmInstruction *instr);
    std::string getName();
    size_t getInstrCount();
    AsmInstruction *getInstrAt(size_t i);
    void print();
private:
    std::string name = "";
    std::vector<AsmInstruction *> block;
};

class AsmFunction {
public:
    explicit AsmFunction(std::string name);
    void addBlock(AsmBlock *block);
    std::string getName();
    size_t getBlockCount();
    AsmBlock *getBlockAt(size_t i);
    void print();
private:
    std::string name = "";
    std::vector<AsmBlock *> blocks;
};

class AsmFile {
public:
    explicit AsmFile(std::string name);
    void addFunction(AsmFunction *func);
    std::string getName();
    size_t getFunctionCount();
    AsmFunction *getFunctionAt(size_t pos);
    void print();
private:
    std::string name = "";
    std::vector<AsmFunction *> functions;
};

