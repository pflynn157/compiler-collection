//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <ast/Types.hpp>
#include <ast/Expression.hpp>

class AstExpression;
class AstID;
class AstInt;

// Represents an AST statement
class AstStatement : public AstNode {
public:
    explicit AstStatement() : AstNode(V_AstType::None) {}
    explicit AstStatement(V_AstType type) : AstNode(type) {}
    
    void setExpression(std::shared_ptr<AstExpression> expr) { this->expr = expr; }
    std::shared_ptr<AstExpression> getExpression() { return expr; }
    bool hasExpression() {
        if (expr) return true;
        return false;
    }
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
private:
    std::shared_ptr<AstExpression> expr = nullptr;
};

// Represents an AST expression statement
// This is basically the same as a statement
class AstExprStatement : public AstStatement {
public:
    explicit AstExprStatement() : AstStatement(V_AstType::ExprStmt) {}
    
    void setDataType(AstDataType *dataType) {
        this->dataType = dataType;
    }
    
    AstDataType *getDataType() { return dataType; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    AstDataType *dataType;
};

// Represents a function call statement
class AstFuncCallStmt : public AstStatement {
public:
    explicit AstFuncCallStmt(std::string name) : AstStatement(V_AstType::FuncCallStmt) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    void print();
    std::string dot(std::string parent) override;
private:
    std::string name = "";
};

// Represents a return statement
class AstReturnStmt : public AstStatement {
public:
    explicit AstReturnStmt() : AstStatement(V_AstType::Return) {}
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a variable declaration
class AstVarDec : public AstStatement {
public:
    explicit AstVarDec(std::string name, AstDataType *dataType) : AstStatement(V_AstType::VarDec) {
        this->name = name;
        this->dataType = dataType;
    }
    
    void setDataType(AstDataType *dataType) { this->dataType = dataType; }
    void setPtrSize(std::shared_ptr<AstExpression> size) { this->size = size; }
    
    std::string getName() { return name; }
    AstDataType *getDataType() { return dataType; }
    std::shared_ptr<AstExpression> getPtrSize() { return size; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::shared_ptr<AstExpression> size = nullptr;
    AstDataType *dataType;
};

// Represents a structure declaration
class AstStructDec : public AstStatement {
public:
    explicit AstStructDec(std::string varName, std::string structName) : AstStatement(V_AstType::StructDec) {
        this->varName = varName;
        this->structName = structName;
    }
    
    void setNoInit(bool init) { noInit = init; }
    
    std::string getVarName() { return varName; }
    std::string getStructName() { return structName; }
    bool isNoInit() { return noInit; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::string varName = "";
    std::string structName = "";
    bool noInit = false;
};

// Represents a conditional statement
class AstIfStmt : public AstStatement {
public:
    explicit AstIfStmt() : AstStatement(V_AstType::If) {}
    
    void setTrueBlock(std::shared_ptr<AstBlock> block) { trueBlock = block; }
    void setFalseBlock(std::shared_ptr<AstBlock> block) { falseBlock = block; }
    
    std::shared_ptr<AstBlock> getTrueBlock() { return trueBlock; }
    std::shared_ptr<AstBlock> getFalseBlock() { return falseBlock; }
    
    void print(int indent);
    std::string dot(std::string parent) override;
private:
    std::shared_ptr<AstBlock> trueBlock = nullptr;
    std::shared_ptr<AstBlock> falseBlock = nullptr;
};

// Represents a while statement
class AstWhileStmt : public AstStatement {
public:
    explicit AstWhileStmt() : AstStatement(V_AstType::While) {}
    
    void setBlock(std::shared_ptr<AstBlock> block) { this->block = block; }
    std::shared_ptr<AstBlock> getBlock() { return block; }
    
    void print(int indent = 0);
    std::string dot(std::string parent) override;
private:
    std::shared_ptr<AstBlock> block = nullptr;
};

// Represents a break statement for a loop
class AstBreak : public AstStatement {
public:
    explicit AstBreak() : AstStatement(V_AstType::Break) {}
    void print();
    std::string dot(std::string parent) override;
};

// Represents a continue statement for a loop
class AstContinue : public AstStatement {
public:
    explicit AstContinue() : AstStatement(V_AstType::Continue) {}
    void print();
    std::string dot(std::string parent) override;
};

