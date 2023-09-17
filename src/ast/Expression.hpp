//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include <ast/Types.hpp>

// Represents an AST expression
class AstExpression : public AstNode {
public:
    explicit AstExpression() : AstNode(V_AstType::None) {}
    explicit AstExpression(V_AstType type) : AstNode(type) {}
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
};

// Holds a list of expressions
class AstExprList : public AstExpression {
public:
    AstExprList() : AstExpression(V_AstType::ExprList) {}
    
    void addExpression(std::shared_ptr<AstExpression> expr) { list.push_back(expr); }
    std::vector<std::shared_ptr<AstExpression>> getList() { return list; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::vector<std::shared_ptr<AstExpression>> list;
};

// Represents the base of operators
class AstOp : public AstExpression {
public:
    bool isBinaryOp() { return isBinary; }

    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
protected:
    bool isBinary = true;
};

// Represents the base of a unary expression
class AstUnaryOp : public AstOp {
public:
    AstUnaryOp() {
        isBinary = false;
    }

    void setVal(std::shared_ptr<AstExpression> val) { this->val = val; }
    std::shared_ptr<AstExpression> getVal() { return val; }
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
protected:
    std::shared_ptr<AstExpression> val;
};

// Represents a negate expression
class AstNegOp : public AstUnaryOp {
public:
    AstNegOp() {
        this->type = V_AstType::Neg;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the base of a binary expression
class AstBinaryOp : public AstOp {
public:
    void setLVal(std::shared_ptr<AstExpression> lval) { this->lval = lval; }
    void setRVal(std::shared_ptr<AstExpression> rval) { this->rval = rval; }
    void setPrecedence(int p) { this->precedence = p; }
    
    std::shared_ptr<AstExpression> getLVal() { return lval; }
    std::shared_ptr<AstExpression> getRVal() { return rval; }
    int getPrecedence() { return precedence; }
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
protected:
    std::shared_ptr<AstExpression> lval;
    std::shared_ptr<AstExpression> rval;
    int precedence = 0;
};

// Represents an assignment operation
class AstAssignOp : public AstBinaryOp {
public:
    explicit AstAssignOp() {
        this->type = V_AstType::Assign;
        this->precedence = 16;
    }
    
    explicit AstAssignOp(std::shared_ptr<AstExpression> lval, std::shared_ptr<AstExpression> rval) {
        this->type = V_AstType::Assign;
        this->precedence = 16;
        this->lval = lval;
        this->rval = rval;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an add operation
class AstAddOp : public AstBinaryOp {
public:
    AstAddOp() {
        this->type = V_AstType::Add;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a subtraction operation
class AstSubOp : public AstBinaryOp {
public:
    AstSubOp() {
        this->type = V_AstType::Sub;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a multiplication operation
class AstMulOp : public AstBinaryOp {
public:
    AstMulOp() {
        this->type = V_AstType::Mul;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
class AstDivOp : public AstBinaryOp {
public:
    AstDivOp() {
        this->type = V_AstType::Div;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the modulus operation
class AstModOp : public AstBinaryOp {
public:
    AstModOp() {
        this->type = V_AstType::Mod;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
class AstAndOp : public AstBinaryOp {
public:
    AstAndOp() {
        this->type = V_AstType::And;
        this->precedence = 8;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an or operation
class AstOrOp : public AstBinaryOp {
public:
    AstOrOp() {
        this->type = V_AstType::Or;
        this->precedence = 10;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a xor operation
class AstXorOp : public AstBinaryOp {
public:
    AstXorOp() {
        this->type = V_AstType::Xor;
        this->precedence = 9;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an equal-to operation
class AstEQOp : public AstBinaryOp {
public:
    AstEQOp() {
        this->type = V_AstType::EQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a not-equal-to operation
class AstNEQOp : public AstBinaryOp {
public:
    AstNEQOp() {
        this->type = V_AstType::NEQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than operation
class AstGTOp : public AstBinaryOp {
public:
    AstGTOp() {
        this->type = V_AstType::GT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than operation
class AstLTOp : public AstBinaryOp {
public:
    AstLTOp() {
        this->type = V_AstType::LT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than-or-equal operation
class AstGTEOp : public AstBinaryOp {
public:
    AstGTEOp() {
        this->type = V_AstType::GTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than-or-equal operation
class AstLTEOp : public AstBinaryOp {
public:
    AstLTEOp() {
        this->type = V_AstType::LTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical AND operation
class AstLogicalAndOp : public AstBinaryOp {
public:
    AstLogicalAndOp() {
        this->type = V_AstType::LogicalAnd;
        this->precedence = 11;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical OR operation
class AstLogicalOrOp : public AstBinaryOp {
public:
    AstLogicalOrOp() {
        this->type = V_AstType::LogicalOr;
        this->precedence = 12;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a character literal
class AstChar : public AstExpression {
public:
    explicit AstChar(char val) : AstExpression(V_AstType::CharL) {
        this->val = val;
    }
    
    char getValue() { return val; }
    void print();
    std::string dot(std::string parent) override;
private:
    char val = 0;
};

// Represents a byte literal
// TODO: Remove
class AstI8 : public AstExpression {
public:
    explicit AstI8(uint8_t val) : AstExpression(V_AstType::I8L) {
        this->val = val;
    }
    
    uint8_t getValue() { return val; }
    void print();
private:
    uint8_t val = 0;
};

// Represents a word literal
// TODO: Remove
class AstI16 : public AstExpression {
public:
    explicit AstI16(uint16_t val) : AstExpression(V_AstType::I16L) {
        this->val = val;
    }
    
    uint16_t getValue() { return val; }
    void print();
private:
    uint16_t val = 0;
};

// Represents an integer literal
// TODO: Convert to uint64, rename AstInt
class AstI32 : public AstExpression {
public:
    explicit AstI32(uint64_t val) : AstExpression(V_AstType::I32L) {
        this->val = val;
    }
    
    void setValue(uint64_t val) { this->val = val; }
    
    uint64_t getValue() { return val; }
    void print();
    std::string dot(std::string parent) override;
private:
    uint64_t val = 0;
};

// Represents a QWord literal
// TODO: Remove
class AstI64 : public AstExpression {
public:
    explicit AstI64(uint64_t val) : AstExpression(V_AstType::I64L) {
        this->val = val;
    }
    
    uint64_t getValue() { return val; }
    void print();
private:
    uint64_t val = 0;
};

// Represents a string literal
class AstString : public AstExpression {
public:
    explicit AstString(std::string val) : AstExpression(V_AstType::StringL) {
        this->val = val;
    }
    
    std::string getValue() { return val; }
    void print();
    std::string dot(std::string parent) override;
private:
    std::string val = "";
};

// Represents a variable reference
class AstID: public AstExpression {
public:
    explicit AstID(std::string val) : AstExpression(V_AstType::ID) {
        this->val = val;
    }
    
    std::string getValue() { return val; }
    void print();
    std::string dot(std::string parent) override;
private:
    std::string val = "";
};

// Represents an array access
class AstArrayAccess : public AstExpression {
public:
    explicit AstArrayAccess(std::string val) : AstExpression(V_AstType::ArrayAccess) {
        this->val = val;
    }
    
    void setIndex(std::shared_ptr<AstExpression> index) { this->index = index; }
    
    std::string getValue() { return val; }
    std::shared_ptr<AstExpression> getIndex() { return index; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::string val = "";
    std::shared_ptr<AstExpression> index;
};

// Represents a structure access
class AstStructAccess : public AstExpression {
public:
    explicit AstStructAccess(std::string var, std::string member) : AstExpression(V_AstType::StructAccess) {
        this->var = var;
        this->member = member;
    }

    std::string getName() { return var; }
    std::string getMember() { return member; }

    void print();
    std::string dot(std::string parent) override;
private:
    std::string var = "";
    std::string member = "";
};

// Represents a function call
class AstFuncCallExpr : public AstExpression {
public:
    explicit AstFuncCallExpr(std::string name) : AstExpression(V_AstType::FuncCallExpr) {
        this->name = name;
    }
    
    void setArgExpression(std::shared_ptr<AstExpression> expr) { this->expr = expr; }
    std::shared_ptr<AstExpression> getArgExpression() { return expr; }
    std::string getName() { return name; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::shared_ptr<AstExpression> expr;
    std::string name = "";
};

