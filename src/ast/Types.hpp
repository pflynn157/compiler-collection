//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>

//
// Contains the variants for all AST nodes
//
enum class V_AstType {
    None,
    
    // Global Statements
    ExternFunc,
    Func,
    StructDef,
    Block,
    
    // Statements
    Return,
    ExprStmt,
    
    FuncCallStmt,
    FuncCallExpr,
    
    VarDec,
    StructDec,
    
    If,
    While,
    
    Break,
    Continue,
    
    // Operators
    Neg,
    
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    
    And,
    Or,
    Xor,
    
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
    
    LogicalAnd,
    LogicalOr,
    
    // Literals and identifiers
    CharL,
    I8L,
    I16L,
    I32L,
    I64L,
    StringL,
    ID,
    ArrayAccess,
    StructAccess,
    
    // Expression list
    ExprList,
    
    // Data types
    Void,
    Bool,
    Char,
    Int8,
    Int16,
    Int32,
    Int64,
    String,
    Ptr,
    Struct
};

//
// The base of all AST nodes
//
class AstNode {
public:
    explicit AstNode() {}
    explicit AstNode(V_AstType type) {
        this->type = type;
    }
    
    V_AstType getType() { return type; }
    
    virtual void print() {}
protected:
    V_AstType type = V_AstType::None;
};

//
// The base of all AST data types
//
class AstDataType : public AstNode {
public:
    explicit AstDataType(V_AstType type) : AstNode(type) {}
    explicit AstDataType(V_AstType type, bool _isUnsigned) : AstNode(type) {
        this->_isUnsigned = _isUnsigned;
    }
    
    void setUnsigned(bool _isUnsigned) { this->_isUnsigned = _isUnsigned; }
    
    bool isUnsigned() { return _isUnsigned; }
    
    void print() override;
protected:
    bool _isUnsigned = false;
};

// Represents a pointer type
class AstPointerType : public AstDataType {
public:
    explicit AstPointerType(AstDataType *baseType) : AstDataType(V_AstType::Ptr) {
        this->baseType = baseType;
    }
    
    AstDataType *getBaseType() { return baseType; }
    
    void print() override;
protected:
    AstDataType *baseType = nullptr;
};

// Represents a structure type
class AstStructType : public AstDataType {
public:
    explicit AstStructType(std::string name) : AstDataType(V_AstType::Struct) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    
    void print() override;
protected:
    std::string name = "";
};

struct Var {
    explicit Var() {}
    explicit Var(AstDataType *type, std::string name = "") {
        this->type = type;
        this->name = name;
    }

    std::string name;
    AstDataType *type;
};

// Forward declarations
class AstExpression;
class AstStatement;

// Represents a block
class AstBlock : public AstNode {
public:
    AstBlock() : AstNode(V_AstType::Block) {}

    void addStatement(AstStatement *stmt);
    void addStatements(std::vector<AstStatement *> block);
    std::vector<AstStatement *> getBlock();
    
    size_t getBlockSize();
    AstStatement *getStatementAt(size_t i);
    
    void removeAt(size_t pos);
    void insertAt(AstStatement *stmt, size_t pos);
    
    void addSymbol(std::string name, AstDataType *dataType);
    void mergeSymbols(AstBlock *parent);
    std::map<std::string, AstDataType *> getSymbolTable();
    AstDataType *getDataType(std::string name);
    
    bool isVar(std::string name);
    
    void print(int indent = 4);
    std::string dot(std::string parent);
protected:
    std::vector<AstStatement *> block;
    std::map<std::string, AstDataType *> symbolTable;
    std::vector<std::string> vars;
};

// Represents a struct
class AstStruct : public AstNode {
public:
    explicit AstStruct(std::string name) : AstNode(V_AstType::StructDef) {
        this->name = name;
    }
    
    void addItem(Var var, AstExpression *defaultExpression) {
        items.push_back(var);
        defaultExpressions[var.name] = defaultExpression;
        
        switch (var.type->getType()) {
            case V_AstType::Char:
            case V_AstType::Int8: size += 1; break;
            case V_AstType::Int16: size += 2; break;
            case V_AstType::Bool:
            case V_AstType::Int32: size += 4; break;
            case V_AstType::String:
            case V_AstType::Ptr:
            case V_AstType::Struct:
            case V_AstType::Int64: size += 8; break;
            
            default: {}
        }
    }
    
    std::string getName() { return name; }
    std::vector<Var> getItems() { return items; }
    int getSize() { return size; }
    
    AstExpression *getDefaultExpression(std::string name) {
        return defaultExpressions[name];
    }
    
    void print();
    std::string dot(std::string parent);
private:
    std::string name;
    std::vector<Var> items;
    std::map<std::string, AstExpression*> defaultExpressions;
    int size = 0;
};

