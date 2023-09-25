//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>

namespace LLIR {

//
// Enumerations
//
enum class X86Type {
    None,        // Always translates to NOP
    
    Label,
    LabelRef,
    Global,
    Extern,
    
    Push,
    Mov,
    Movsx,
    Lea,
    Call,
    Cdq,
    Leave,
    Ret,
    
    Add,
    Sub,
    And,
    Or,
    Xor,
    Cmp,
    IMul,
    IDiv,
    
    Jmp,
    Je,
    Jne,
    Jg,
    Jl,
    Jge,
    Jle,
    
    Reg8,
    Reg8H,
    Reg16,
    Reg32,
    Reg64,
    RegPtr,
    
    Imm,
    Mem,
    String
};

enum class X86Reg {
    AX,
    BX,
    CX,
    DX,
    SI,
    DI,
    SP,
    BP,
    R8, R9, R10, R11, R12, R13, R14, R15
};

enum class AsmType {
    GAS,
    NASM
};

// Forward declarations
class X86Data;
class X86Instr;
class X86Operand;

//
// Represents an x86 data item
//
class X86Data {
public:
    explicit X86Data(std::string name, std::string val) {
        this->name = name;
        this->val = val;
    }
    
    std::string print() {
        return name + ": .string \"" + val + "\"";
    }
private:
    std::string name = "";
    std::string val = "";
};

//
// Represents an X86 operand
//
class X86Operand {
public:
    explicit X86Operand(X86Type type) {
        this->type = type;
    }
    
    X86Type getType() { return type; }
    
    virtual std::string print() { return ""; }
protected:
    X86Type type = X86Type::None;
};

//
// Represents an X86 instruction
// This is the base of all X86 instructions
//
class X86Instr {
public:
    explicit X86Instr(X86Type type) {
        this->type = type;
    }
    
    ~X86Instr() {
        if (op1) delete op1;
        if (op2) delete op2;
    }
    
    void setOperand1(X86Operand *op1) { this->op1 = op1; }
    void setOperand2(X86Operand *op2) { this->op2 = op2; }
    
    X86Type getType() { return type; }
    X86Operand *getOperand1() { return op1; }
    X86Operand *getOperand2() { return op2; }
    
    virtual std::string print() { return "nop"; }
protected:
    X86Type type = X86Type::None;
    X86Operand *op1 = nullptr;
    X86Operand *op2 = nullptr;
};

// Represents an x86 file
class X86File {
public:
    explicit X86File(std::string name) {
        this->name = name;
    }
    
    ~X86File() {
        for (X86Data *d : data) {
            if (d) delete d;
        }
        for (X86Instr *i : code) {
            if (i) delete i;
        }
    }
    
    void addData(X86Data *d) { data.push_back(d); }
    void addCode(X86Instr *c) { code.push_back(c); }
    
    std::string print(AsmType type = AsmType::GAS);
private:
    std::string name = "";
    std::vector<X86Data *> data;
    std::vector<X86Instr *> code;
};

//
// The following are the X86 instruction representations
//

// The labels
class X86Label : public X86Instr {
public:
    explicit X86Label(std::string name) : X86Instr(X86Type::Label) {
        this->name = name;
    }
    
    std::string print() {
        return name + ":";
    }
protected:
    std::string name = "";
};

// Globally visible functions
class X86GlobalFunc : public X86Instr {
public:
    explicit X86GlobalFunc(std::string name) : X86Instr(X86Type::Global) {
        this->name = name;
    }
    
    std::string print();
protected:
    std::string name = "";
};

// A push instruction
class X86Push : public X86Instr {
public:
    explicit X86Push() : X86Instr(X86Type::Push) {}
    explicit X86Push(X86Operand *op) : X86Instr(X86Type::Push) {
        op1 = op;
    }
    
    std::string print();
};

// An MOV instruction
class X86Mov : public X86Instr {
public:
    explicit X86Mov(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Mov) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An MOVSX instruction
class X86Movsx : public X86Instr {
public:
    explicit X86Movsx(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Movsx) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An LEA instruction
class X86Lea : public X86Instr {
public:
    explicit X86Lea(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Lea) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An ADD instruction
class X86Add : public X86Instr {
public:
    explicit X86Add(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Add) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// A SUB instruction
class X86Sub : public X86Instr {
public:
    explicit X86Sub(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Sub) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An AND instruction
class X86And : public X86Instr {
public:
    explicit X86And(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::And) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An OR instruction
class X86Or : public X86Instr {
public:
    explicit X86Or(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Or) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// A XOR instruction
class X86Xor : public X86Instr {
public:
    explicit X86Xor(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Xor) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// A CMP instruction
class X86Cmp : public X86Instr {
public:
    explicit X86Cmp(X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::Cmp) {
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
};

// An IMUL instruction
class X86IMul : public X86Instr {
public:
    explicit X86IMul(X86Operand *dest, X86Operand *op1, X86Operand *op2) : X86Instr(X86Type::IMul) {
        this->dest = dest;
        this->op1 = op1;
        this->op2 = op2;
    }
    
    std::string print();
private:
    X86Operand *dest;
};

// An IDIV instruction
class X86IDiv : public X86Instr {
public:
    explicit X86IDiv(X86Operand *op1) : X86Instr(X86Type::IDiv) {
        this->op1 = op1;
    }
    
    std::string print();
};

class X86Jmp : public X86Instr {
public:
    explicit X86Jmp(X86Operand *lbl, X86Type jType) : X86Instr(jType) {
        this->op1 = lbl;
    }
    
    std::string print();
};

// a CALL instruction
class X86Call : public X86Instr {
public:
    explicit X86Call(std::string name) : X86Instr(X86Type::Call) {
        this->name = name;
    }
    
    std::string print() { return "call " + name; }
private:
    std::string name = "";
};

// a CDQ instruction
class X86Cdq : public X86Instr {
public:
    X86Cdq() : X86Instr(X86Type::Cdq) {}
    std::string print() { return "cdq"; }
};

// a LEAVE instruction
class X86Leave : public X86Instr {
public:
    X86Leave() : X86Instr(X86Type::Leave) {}
    std::string print() { return "leave"; }
};

// a RET instruction
class X86Ret : public X86Instr {
public:
    X86Ret() : X86Instr(X86Type::Ret) {}
    std::string print() { return "ret"; }
};

//
// The base of all operands
//

// Represents a reference to a label
class X86LabelRef : public X86Operand {
public:
    explicit X86LabelRef(std::string name) : X86Operand(X86Type::LabelRef) {
        this->name = name;
    }
    
    std::string print();
protected:
    std::string name = "";
};

// Represents an immediate value
class X86Imm : public X86Operand {
public:
    explicit X86Imm(uint64_t imm) : X86Operand(X86Type::Imm) {
        this->value = imm;
    }
    
    int64_t getValue() { return value; }
    void setValue(int64_t imm) { value = imm; }
    
    std::string print();
private:
    int64_t value = 0;
};

// Represents a 8-bit register
class X86Reg8 : public X86Operand {
public:
    explicit X86Reg8(X86Reg regType) : X86Operand(X86Type::Reg8) {
        this->regType = regType;
    }
    
    X86Reg getType() { return regType; }
    std::string print();
protected:
    X86Reg regType;
};

// Represents a 16-bit register
class X86Reg16 : public X86Operand {
public:
    explicit X86Reg16(X86Reg regType) : X86Operand(X86Type::Reg16) {
        this->regType = regType;
    }
    
    X86Reg getType() { return regType; }
    std::string print();
protected:
    X86Reg regType;
};

// Represents a 32-bit register
class X86Reg32 : public X86Operand {
public:
    explicit X86Reg32(X86Reg regType) : X86Operand(X86Type::Reg32) {
        this->regType = regType;
    }
    
    X86Reg getType() { return regType; }
    std::string print();
protected:
    X86Reg regType;
};

// Represents a 64-bit register
class X86Reg64 : public X86Operand {
public:
    explicit X86Reg64(X86Reg regType) : X86Operand(X86Type::Reg64) {
        this->regType = regType;
    }
    
    X86Reg getType() { return regType; }
    std::string print();
protected:
    X86Reg regType;
};

// Represents a  pointer
class X86RegPtr : public X86Operand {
public:
    explicit X86RegPtr(X86Reg regType) : X86Operand(X86Type::RegPtr) {
        this->regType = regType;
    }
    
    void setSizeAttr(std::string sizeAttr) { this->sizeAttr = sizeAttr; }
    
    X86Reg getType() { return regType; }
    std::string print();
protected:
    X86Reg regType;
    std::string sizeAttr = "";
};

// Represents a memory location
class X86Mem : public X86Operand {
public:
    explicit X86Mem(X86Operand *base, X86Operand *offset) : X86Operand(X86Type::Mem) {
        this->base = base;
        this->offset = offset;
    }
    
    explicit X86Mem(X86Operand *offset) : X86Operand(X86Type::Mem) {
        this->offset = offset;
        this->base = new X86Reg64(X86Reg::BP);
    }
    
    X86Operand *getOffset() { return offset; }
    
    void setSizeAttr(std::string sizeAttr) { this->sizeAttr = sizeAttr; }
    
    std::string print();
private:
    X86Operand *base, *offset;
    std::string sizeAttr = "";
};

// Represents a string value
class X86String : public X86Operand {
public:
    explicit X86String(std::string val) : X86Operand(X86Type::String) {
        this->value = val;
    }
    
    std::string print();
private:
    std::string value = "";
};

} // end namespace llir
