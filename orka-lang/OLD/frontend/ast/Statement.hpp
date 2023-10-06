//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>

#include <ast/Types.hpp>
#include <ast/Expression.hpp>

class AstExpression;
class AstID;
class AstInt;

// Represents an AST statement
class AstStatement {
public:
    explicit AstStatement() {}
    explicit AstStatement(AstType type) {
        this->type = type;
    }
    
    void addExpression(AstExpression *expr) {
        expressions.push_back(expr);
    }
    
    int getExpressionCount() {
        return expressions.size();
    }
    
    void clearExpressions() {
        expressions.clear();
    }
    
    std::vector<AstExpression *> getExpressions() { return expressions; }
    AstExpression *getExpression() { return expressions.at(0); }
    AstType getType() { return type; }
    virtual void print() {}
private:
    std::vector<AstExpression *> expressions;
    AstType type = AstType::EmptyAst;
};

// Represents a function call statement
class AstFuncCallStmt : public AstStatement {
public:
    explicit AstFuncCallStmt(std::string name) : AstStatement(AstType::FuncCallStmt) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    void print();
private:
    std::string name = "";
};

// Represents a return statement
class AstReturnStmt : public AstStatement {
public:
    explicit AstReturnStmt() : AstStatement(AstType::Return) {}
    void print();
};

// Represents a variable declaration
class AstVarDec : public AstStatement {
public:
    explicit AstVarDec(std::string name, DataType dataType) : AstStatement(AstType::VarDec) {
        this->name = name;
        this->dataType = dataType;
    }
    
    void setDataType(DataType dataType) { this->dataType = dataType; }
    void setPtrType(DataType dataType) { this->ptrType = dataType; }
    void setPtrSize(AstExpression *size) { this->size = size; }
    
    std::string getName() { return name; }
    DataType getDataType() { return dataType; }
    DataType getPtrType() { return ptrType; }
    AstExpression *getPtrSize() { return size; }
    
    void print();
private:
    std::string name = "";
    AstExpression *size = nullptr;
    DataType dataType = DataType::Void;
    DataType ptrType = DataType::Void;
};

// Represents a structure declaration
class AstStructDec : public AstStatement {
public:
    explicit AstStructDec(std::string varName, std::string structName) : AstStatement(AstType::StructDec) {
        this->varName = varName;
        this->structName = structName;
    }
    
    void setNoInit(bool init) { noInit = init; }
    
    std::string getVarName() { return varName; }
    std::string getStructName() { return structName; }
    bool isNoInit() { return noInit; }
    
    void print();
private:
    std::string varName = "";
    std::string structName = "";
    bool noInit = false;
};

// Represents a variable assignment
class AstVarAssign : public AstStatement {
public:
    explicit AstVarAssign(std::string name) : AstStatement(AstType::VarAssign) {
        this->name = name;
    }
    
    void setDataType(DataType dataType) { this->dataType = dataType; }
    void setPtrType(DataType dataType) { this->ptrType = dataType; }
    
    std::string getName() { return name; }
    DataType getDataType() { return dataType; }
    DataType getPtrType() { return ptrType; }
    
    void print();
private:
    std::string name = "";
    DataType dataType = DataType::Void;
    DataType ptrType = DataType::Void;
};

// Represents an array assignment
class AstArrayAssign : public AstStatement {
public:
    explicit AstArrayAssign(std::string name) : AstStatement(AstType::ArrayAssign) {
        this->name = name;
    }
    
    void setDataType(DataType dataType) { this->dataType = dataType; }
    void setPtrType(DataType dataType) { this->ptrType = dataType; }
    
    std::string getName() { return name; }
    DataType getDataType() { return dataType; }
    DataType getPtrType() { return ptrType; }
    
    void print();
private:
    std::string name = "";
    DataType dataType = DataType::Void;
    DataType ptrType = DataType::Void;
};

// Represents a struct assignment
class AstStructAssign : public AstStatement {
public:
    explicit AstStructAssign(std::string name, std::string member) : AstStatement(AstType::StructAssign) {
        this->name = name;
        this->member = member;
    }
    
    void setMemberType(DataType memberType) {
        this->memberType = memberType;
    }
    
    std::string getName() { return name; }
    std::string getMember() { return member; }
    DataType getMemberType() { return memberType; }
    
    void print();
private:
    std::string name = "";
    std::string member = "";
    DataType memberType = DataType::Void;
};

// Represents a statement with a sub-block
class AstBlockStmt : public AstStatement {
public:
    explicit AstBlockStmt(AstType type) : AstStatement(type) {
        block = new AstBlock;
    }
    
    void addStatement(AstStatement *stmt) { block->addStatement(stmt); }
    
    AstBlock *getBlockStmt() { return block; }
    std::vector<AstStatement *> getBlock() { return block->getBlock(); }
protected:
    AstBlock *block;
};

// Represents a conditional statement
class AstIfStmt : public AstBlockStmt {
public:
    explicit AstIfStmt() : AstBlockStmt(AstType::If) {}
    
    void addBranch(AstStatement *stmt) { branches.push_back(stmt); }
    std::vector<AstStatement *> getBranches() { return branches; }
    
    void print();
private:
    std::vector<AstStatement *> branches;
};

class AstElifStmt : public AstBlockStmt {
public:
    explicit AstElifStmt() : AstBlockStmt(AstType::Elif) {}
    
    void print();
};

class AstElseStmt : public AstBlockStmt {
public:
    explicit AstElseStmt() : AstBlockStmt(AstType::Else) {}
    
    void print();
};

// Represents a while statement
class AstWhileStmt : public AstBlockStmt {
public:
    explicit AstWhileStmt() : AstBlockStmt(AstType::While) {}
    
    void print();
};

// Represents an infinite loop statement
class AstRepeatStmt : public AstBlockStmt {
public:
    explicit AstRepeatStmt() : AstBlockStmt(AstType::Repeat) {}
    
    void print();
};

// Represents a for loop
class AstForStmt : public AstBlockStmt {
public:
    explicit AstForStmt() : AstBlockStmt(AstType::For) {}
    
    void setIndex(AstID *indexVar) { this->indexVar = indexVar; }
    void setStep(int amount) { step->setValue(amount); }
    void setStartBound(AstExpression *expr) { startBound = expr; }
    void setEndBound(AstExpression *expr) { endBound = expr; }
    
    AstID *getIndex() { return indexVar; }
    AstInt *getStep() { return step; }
    AstExpression *getStartBound() { return startBound; }
    AstExpression *getEndBound() { return endBound; }
    
    void print();
private:
    AstID *indexVar;
    AstExpression *startBound, *endBound;
    AstInt *step = new AstInt(1);
};

// Represents a for-all loop
class AstForAllStmt : public AstBlockStmt {
public:
    explicit AstForAllStmt() : AstBlockStmt(AstType::ForAll) {}
    
    void setIndex(AstID *indexVar) { this->indexVar = indexVar; }
    void setArray(AstID *arrayVar) { this->arrayVar = arrayVar; }
    
    AstID *getIndex() { return indexVar; }
    AstID *getArray() { return arrayVar; }
    
    void print();
private:
    AstID *indexVar, *arrayVar;
};

// Represents a break statement for a loop
class AstBreak : public AstStatement {
public:
    explicit AstBreak() : AstStatement(AstType::Break) {}
    void print();
};

// Represents a continue statement for a loop
class AstContinue : public AstStatement {
public:
    explicit AstContinue() : AstStatement(AstType::Continue) {}
    void print();
};

// Represents a block end
class AstEnd : public AstStatement {
public:
    explicit AstEnd() : AstStatement(AstType::End) {}
    void print();
};

