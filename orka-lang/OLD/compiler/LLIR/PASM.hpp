// The definition for portable assembly IR (PASM)
#pragma once

#include <string>
#include <vector>

class PASMFile;
class PASMInstruction;
class PASMOperand;

enum class PASM {
    None,
    
    Label,
    Func,
    
    // Instructions
    NOP,
    LI,
    RET,
    
    // Operands
    R8,
    R16,
    R32,
    R64,
    Imm
};

//
// Represents a portable assembly file
//
class PASMFile {
public:
    explicit PASMFile(std::string name) {
        this->name = name;
    }
    
    void addCode(PASMInstruction *instr) {
        code.push_back(instr);
    }
    
    std::vector<PASMInstruction *> getCode() {
        return code;
    }
    
    std::string getName() { return name; }
    
    void debug();
private:
    std::string name = "";
    std::vector<PASMInstruction *> code;
};

//
// Represents an instruction
//
class PASMInstruction {
public:
    explicit PASMInstruction(PASM opType) {
        this->opType = opType;
    }
    
    void setOperand1(PASMOperand *op) { op1 = op; }
    void setOperand2(PASMOperand *op) { op2 = op; }
    void setOperand3(PASMOperand *op) { op3 = op; }
    
    PASMOperand *getOperand1() { return op1; }
    PASMOperand *getOperand2() { return op2; }
    PASMOperand *getOperand3() { return op3; }
    
    PASM getOpType() { return opType; }
    
    virtual void debug();
protected:
    PASM opType = PASM::None;
    PASMOperand *op1 = nullptr;
    PASMOperand *op2 = nullptr;
    PASMOperand *op3 = nullptr;
};

class PASMLabel : public PASMInstruction {
public:
    explicit PASMLabel(std::string name) : PASMInstruction(PASM::Label) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    
    void debug();
private:
    std::string name = "";
};

class PASMFunc : public PASMInstruction {
public:
    explicit PASMFunc(std::string name, int stackSize) : PASMInstruction(PASM::Func) {
        this->name = name;
        this->stackSize = stackSize;
    }
    
    std::string getName() { return name; }
    int getStackSize() { return stackSize; }
    
    void debug();
private:
    std::string name;
    int stackSize = 0;
};

//
// Represents an operand
//
class PASMOperand {
public:
    explicit PASMOperand(PASM opType) {
        this->opType = opType;
    }
    
    PASM getOpType() { return opType; }
    
    virtual void debug() {}
protected:
    PASM opType = PASM::None;
};

// More specific operands
//
class PASMImm : public PASMOperand {
public:
    explicit PASMImm(int val) : PASMOperand(PASM::Imm) {
        this->val = val;
    }
    
    int getValue() { return val; }
    
    void debug();
private:
    int val = 0;
};

class PASMReg : public PASMOperand {
public:
    explicit PASMReg(int num, PASM regType) : PASMOperand(regType) {
        this->num = num;
    }
    
    int getRegNum() { return num; }
    void debug();
protected:
    int num = 0;
};

class PASMReg8 : public PASMReg {
public:
    explicit PASMReg8(int num) : PASMReg(num, PASM::R8) {}
};

class PASMReg16 : public PASMReg {
public:
    explicit PASMReg16(int num) : PASMReg(num, PASM::R16) {}
};

class PASMReg32 : public PASMReg {
public:
    explicit PASMReg32(int num) : PASMReg(num, PASM::R32) {}
};

class PASMReg64 : public PASMReg {
public:
    explicit PASMReg64(int num) : PASMReg(num, PASM::R64) {}
};

