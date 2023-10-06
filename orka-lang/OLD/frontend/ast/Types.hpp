//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>

enum class AstType {
    EmptyAst,
    ExternFunc,
    Func,
    Return,
    
    FuncCallStmt,
    FuncCallExpr,
    
    VarDec,
    StructDec,
    ClassDec,
    VarAssign,
    ArrayAssign,
    StructAssign,
    Sizeof,
    
    If,
    Elif,
    Else,
    While,
    Repeat,
    For,
    ForAll,
    End,
    
    Break,
    Continue,
    
    Neg,
    
    Add,
    Sub,
    Mul,
    Div,
    
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
    
    BoolL,
    CharL,
    ByteL,
    WordL,
    IntL,
    QWordL,
    FloatL,
    StringL,
    ID,
    ArrayAccess,
    StructAccess
};

enum class DataType {
    Void,
    Bool,
    Char,
    Byte,
    UByte,
    Short,
    UShort,
    Int32,
    UInt32,
    Int64,
    UInt64,
    String,
    Array,
    Ptr,
    Double,
    Float,
    Struct
};

struct Var {
    std::string name;
    DataType type;
    DataType subType;
    std::string typeName;
};

// Represents an ENUM
class AstExpression;

struct EnumDec {
    std::string name;
    DataType type;
    std::map<std::string, AstExpression*> values;
};

// Represents a block
class AstStatement;

class AstBlock {
public:
    void addStatement(AstStatement *stmt) { block.push_back(stmt); }
    void addStatements(std::vector<AstStatement *> block) { this->block = block; }
    std::vector<AstStatement *> getBlock() { return block; }
private:
    std::vector<AstStatement *> block;
};

// Represents a struct
class AstStruct {
public:
    explicit AstStruct(std::string name) {
        this->name = name;
    }
    
    void addItem(Var var, AstExpression *defaultExpression) {
        items.push_back(var);
        defaultExpressions[var.name] = defaultExpression;
    }
    
    std::string getName() { return name; }
    std::vector<Var> getItems() { return items; }
    
    AstExpression *getDefaultExpression(std::string name) {
        return defaultExpressions[name];
    }
    
    void print();
private:
    std::string name;
    std::vector<Var> items;
    std::map<std::string, AstExpression*> defaultExpressions;
};
