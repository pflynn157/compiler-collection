//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

namespace LLIR {

// Represents operand types
enum class OpType {
    None,
    
    Imm,
    Reg,
    Label,
    String,
    
    Mem,
    HReg,        // Hardware register
    AReg,        // Argument register
    PReg         // Pointer register
};

/*! \brief The base for LLIR operands
 *
 * This is the base class for all LLIR operands. This class generally should not be constructed directly;
 * instead, use the derived versions. However, in cases where any type of LLIR operand is acceptable,
 * this class should be used as a parameter/return type.
 */
class Operand {
public:
    /*! \brief Creates a new operand
     *
     * @param type The type of the operand
     */
    explicit Operand(OpType type) {
        this->type = type;
    }
    
    virtual ~Operand() {}
    
    /*! \brief Returns the type of the operand
     *
     */
    OpType getType() { return type; }
    
    virtual void print() {}
protected:
    OpType type = OpType::None;
};

/*! \brief An LLIR immediate value
 *
 * Represents an LLIR immediate operand. This can be of any byte length.
 */
class Imm : public Operand {
public:
    explicit Imm(int64_t imm) : Operand(OpType::Imm) {
        this->imm = imm;
    }
    
    int64_t getValue() { return imm; }
    void setValue(int64_t imm) { this->imm = imm; }
    
    void print();
private:
    int64_t imm = 0;
};

/*! \brief An LLIR virtual register
 *
 * This represents a virtual register in LLIR. You should always use virtual registers for
 * assignments. A virtual register can hold values and represent memory locations, depending
 * on the situation.
 */
class Reg : public Operand {
public:
    explicit Reg(std::string name) : Operand(OpType::Reg) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    
    void print();
private:
    std::string name = "";
};

/*! \brief An LLIR label
 *
 * Represents a label in LLIR. A label should correspond with a Block somewhere in
 * the current function scope. In general, these are used in the branch instructions.
 */
class Label : public Operand {
public:
    explicit Label(std::string name) : Operand(OpType::Label) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    
    void print();
private:
    std::string name = "";
};

/*! \brief An LLIR global string pointer
 *
 * This represents a string pointer in LLIR. A string pointer consists of the name of the
 * pointer, and the value. There should be two copies of this class: one for the module, and
 * the other whenever we need to reference the string.
 */
class StringPtr : public Operand {
public:
    explicit StringPtr(std::string name, std::string val) : Operand(OpType::String) {
        this->name = name;
        this->val = val;
    }
    
    std::string getName() { return name; }
    std::string getValue() { return val; }
    
    void print();
private:
    std::string name = "";
    std::string val = "";
};

//
// NOTE: These are more hardware-specific operands, and should only be used by
// the transform layer and the assembly writers
//

// Represents a memory location
class Mem : public Operand {
public:
    explicit Mem(std::string name) : Operand(OpType::Mem) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    
    void print();
private:
    std::string name = "";
};

// Represents a hardware register
class HReg : public Operand {
public:
    explicit HReg(int num) : Operand(OpType::HReg) {
        this->num = num;
    }
    
    int getNum() { return num; }
    
    void print();
private:
    int num = 0;
};

// Represents a hardware argument register
class AReg : public Operand {
public:
    explicit AReg(int num) : Operand(OpType::AReg) {
        this->num = num;
    }
    
    int getNum() { return num; }
    
    void print();
private:
    int num = 0;
};

// Represents a hardware pointer register
class PReg : public Operand {
public:
    explicit PReg(int num) : Operand(OpType::PReg) {
        this->num = num;
    }
    
    int getNum() { return num; }
    
    void print();
private:
    int num = 0;
};

} // end namespace LLIR
