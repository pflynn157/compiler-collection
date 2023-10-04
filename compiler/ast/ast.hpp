//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>
#include <memory>
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
    Struct,
    Object
};

//
// Attributes needed by some languages
//
enum class Attr {
    Public,
    Protected,
    Private
};

// Forward declarations
struct AstExpression;
struct AstStatement;

//
// The base of all AST nodes
//
struct AstNode {
public:
    explicit AstNode();
    explicit AstNode(V_AstType type);
    virtual void print() {}
    
    V_AstType type = V_AstType::None;
};

//
// AST data types
//

// The base of all AST data types
struct AstDataType : AstNode {
    explicit AstDataType(V_AstType type);
    explicit AstDataType(V_AstType type, bool _isUnsigned);
    void print() override;

    bool is_unsigned = false;
};

// Represents a pointer type
struct AstPointerType : AstDataType {
    explicit AstPointerType(std::shared_ptr<AstDataType> base_type); 
    void print() override;
    
    std::shared_ptr<AstDataType> base_type = nullptr;
};

// Represents a structure type
struct AstStructType : AstDataType {
    explicit AstStructType(std::string name);
    void print() override;
    
    std::string name = "";
};

// Represents an object type
struct AstObjectType : AstDataType {
    explicit AstObjectType();
    void print() override;
};

// Var
struct Var {
    explicit Var();
    explicit Var(std::shared_ptr<AstDataType> type, std::string name = "");
    
    std::string name;
    std::shared_ptr<AstDataType> type;
};

//
// AstStruct
// Represents a struct
//
struct AstStruct : AstNode {
    explicit AstStruct(std::string name);
    
    void addItem(Var var, std::shared_ptr<AstExpression> default_expression);
    
    void print();
    std::string dot(std::string parent);
    
    // Member variables
    std::string name;
    std::vector<Var> items;
    std::map<std::string, std::shared_ptr<AstExpression>> default_expressions;
    int size = 0;
};

//
// Represents an AstBlock
// Blocks hold tables and symbol information
//
struct AstBlock : AstNode {
    AstBlock() : AstNode(V_AstType::Block) {}

    void addStatement(std::shared_ptr<AstStatement> stmt);
    void addStatements(std::vector<std::shared_ptr<AstStatement>> block);
    std::vector<std::shared_ptr<AstStatement>> getBlock();
    
    size_t getBlockSize();
    std::shared_ptr<AstStatement> getStatementAt(size_t i);
    
    void removeAt(size_t pos);
    void insertAt(std::shared_ptr<AstStatement> stmt, size_t pos);
    
    void addSymbol(std::string name, std::shared_ptr<AstDataType> dataType);
    void mergeSymbols(std::shared_ptr<AstBlock> parent);
    std::map<std::string, std::shared_ptr<AstDataType>> getSymbolTable();
    std::shared_ptr<AstDataType> getDataType(std::string name);
    
    bool isVar(std::string name);
    int isConstant(std::string name);
    bool isFunc(std::string name);
    
    void print(int indent = 4);
    std::string dot(std::string parent);
    
    // Members
    std::vector<std::shared_ptr<AstStatement>> block;
    std::map<std::string, std::shared_ptr<AstDataType>> symbolTable;
    std::vector<std::string> vars;
    
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> globalConsts;
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> localConsts;
    std::vector<std::string> funcs;
};

//
// -----------------------------------------------------
// AstExpressions
// -----------------------------------------------------
//

// Represents an AST expression
struct AstExpression : AstNode {
    explicit AstExpression() : AstNode(V_AstType::None) {}
    explicit AstExpression(V_AstType type) : AstNode(type) {}
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
};

// Holds a list of expressions
struct AstExprList : AstExpression {
    AstExprList() : AstExpression(V_AstType::ExprList) {}
    void add_expression(std::shared_ptr<AstExpression> expr) { list.push_back(expr); }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::vector<std::shared_ptr<AstExpression>> list;
};

// Represents the base of operators
struct AstOp : AstExpression {
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    bool is_binary = true;
};

// Represents the base of a unary expression
struct AstUnaryOp : AstOp {
    AstUnaryOp() {
        is_binary = false;
    }
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> value;
};

// Represents a negate expression
struct AstNegOp : AstUnaryOp {
    AstNegOp() {
        this->type = V_AstType::Neg;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the base of a binary expression
struct AstBinaryOp : AstOp {
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> lval;
    std::shared_ptr<AstExpression> rval;
    int precedence = 0;
};

// Represents an assignment operation
struct AstAssignOp : AstBinaryOp {
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
struct AstAddOp : AstBinaryOp {
    AstAddOp() {
        this->type = V_AstType::Add;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a subtraction operation
struct AstSubOp : AstBinaryOp {
    AstSubOp() {
        this->type = V_AstType::Sub;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a multiplication operation
struct AstMulOp : AstBinaryOp {
    AstMulOp() {
        this->type = V_AstType::Mul;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
struct AstDivOp : AstBinaryOp {
    AstDivOp() {
        this->type = V_AstType::Div;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the modulus operation
struct AstModOp : AstBinaryOp {
    AstModOp() {
        this->type = V_AstType::Mod;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
struct AstAndOp : AstBinaryOp {
    AstAndOp() {
        this->type = V_AstType::And;
        this->precedence = 8;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an or operation
struct AstOrOp : AstBinaryOp {
    AstOrOp() {
        this->type = V_AstType::Or;
        this->precedence = 10;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a xor operation
struct AstXorOp : AstBinaryOp {
    AstXorOp() {
        this->type = V_AstType::Xor;
        this->precedence = 9;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an equal-to operation
struct AstEQOp : AstBinaryOp {
    AstEQOp() {
        this->type = V_AstType::EQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a not-equal-to operation
struct AstNEQOp : AstBinaryOp {
    AstNEQOp() {
        this->type = V_AstType::NEQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than operation
struct AstGTOp : AstBinaryOp {
    AstGTOp() {
        this->type = V_AstType::GT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than operation
struct AstLTOp : AstBinaryOp {
    AstLTOp() {
        this->type = V_AstType::LT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than-or-equal operation
struct AstGTEOp : AstBinaryOp {
    AstGTEOp() {
        this->type = V_AstType::GTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than-or-equal operation
struct AstLTEOp : AstBinaryOp {
    AstLTEOp() {
        this->type = V_AstType::LTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical AND operation
struct AstLogicalAndOp : AstBinaryOp {
    AstLogicalAndOp() {
        this->type = V_AstType::LogicalAnd;
        this->precedence = 11;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical OR operation
struct AstLogicalOrOp : AstBinaryOp {
    AstLogicalOrOp() {
        this->type = V_AstType::LogicalOr;
        this->precedence = 12;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a character literal
struct AstChar : AstExpression {
    explicit AstChar(char val) : AstExpression(V_AstType::CharL) {
        this->value = val;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    char value = 0;
};

// Represents a byte literal
// TODO: Remove
struct AstI8 : AstExpression {
    explicit AstI8(uint8_t val) : AstExpression(V_AstType::I8L) {
        this->value = val;
    }
    
    uint8_t getValue() { return value; }
    void print();
    
    uint8_t value = 0;
};

// Represents a word literal
// TODO: Remove
struct AstI16 : AstExpression {
    explicit AstI16(uint16_t val) : AstExpression(V_AstType::I16L) {
        this->value = val;
    }
    
    uint16_t getValue() { return value; }
    void print();

    uint16_t value = 0;
};

// Represents an integer literal
// TODO: Convert to uint64, rename AstInt
struct AstI32 : AstExpression {
    explicit AstI32(uint64_t val) : AstExpression(V_AstType::I32L) {
        this->value = val;
    }
    
    void setValue(uint64_t val) { this->value = val; }
    
    uint64_t getValue() { return value; }
    void print();
    std::string dot(std::string parent) override;
    
    uint64_t value = 0;
};

// Represents a QWord literal
// TODO: Remove
struct AstI64 : AstExpression {
    explicit AstI64(uint64_t val) : AstExpression(V_AstType::I64L) {
        this->value = val;
    }
    
    uint64_t getValue() { return value; }
    void print();
    
    uint64_t value = 0;
};

// Represents a string literal
struct AstString : AstExpression {
    explicit AstString(std::string value) : AstExpression(V_AstType::StringL) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};

// Represents a variable reference
struct AstID: AstExpression {
    explicit AstID(std::string val) : AstExpression(V_AstType::ID) {
        this->value = val;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};

// Represents an array access
struct AstArrayAccess : AstExpression {
    explicit AstArrayAccess(std::string value) : AstExpression(V_AstType::ArrayAccess) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string value = "";
    std::shared_ptr<AstExpression> index;
};

// Represents a structure access
struct AstStructAccess : AstExpression {
    explicit AstStructAccess(std::string var, std::string member) : AstExpression(V_AstType::StructAccess) {
        this->var = var;
        this->member = member;
    }

    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string var = "";
    std::string member = "";
};

// Represents a function call
struct AstFuncCallExpr : AstExpression {
    explicit AstFuncCallExpr(std::string name) : AstExpression(V_AstType::FuncCallExpr) {
        this->name = name;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::shared_ptr<AstExpression> args;
    std::string name = "";
};


//
// -----------------------------------------------------
// AstStatements
// -----------------------------------------------------
//

// Represents an AST statement
struct AstStatement : AstNode {
    explicit AstStatement();
    explicit AstStatement(V_AstType type);
    bool hasExpression();
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> expression = nullptr;
};

// Represents an extern function
struct AstExternFunction : AstStatement {
    explicit AstExternFunction(std::string name) : AstStatement(V_AstType::ExternFunc) {
        this->name = name;
    }
    
    void addArgument(Var arg) { this->args.push_back(arg); }
    
    void print() override;
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstDataType> data_type;
    bool varargs = false;
};

// Represents a function
struct AstFunction : AstStatement {
    explicit AstFunction(std::string name) : AstStatement(V_AstType::Func) {
        this->name = name;
        block = std::make_shared<AstBlock>();
    }
    
    void addStatement(std::shared_ptr<AstStatement> statement) {
        block->addStatement(statement);
    }
    
    void print() override;
    std::string dot(std::string parent) override;

    // Member variables
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block;
    std::shared_ptr<AstDataType> data_type;
    std::string dtName = "";
    
    // These attributes are language specific
    Attr attr = Attr::Public;
    bool routine = false;
};

// Represents an AST expression statement
// This is basically the same as a statement
struct AstExprStatement : AstStatement {
    explicit AstExprStatement() : AstStatement(V_AstType::ExprStmt) {}
    
    void setDataType(std::shared_ptr<AstDataType> dataType) {
        this->dataType = dataType;
    }
    
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::shared_ptr<AstDataType> dataType;
};

// Represents a function call statement
struct AstFuncCallStmt : AstStatement {
    explicit AstFuncCallStmt(std::string name) : AstStatement(V_AstType::FuncCallStmt) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    void print();
    std::string dot(std::string parent) override;
    
    std::string name = "";
    
    // Language-specific attributes
    std::string object_name = "";
};

// Represents a return statement
struct AstReturnStmt : AstStatement {
    explicit AstReturnStmt() : AstStatement(V_AstType::Return) {}
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a variable declaration
struct AstVarDec : AstStatement {
    explicit AstVarDec(std::string name, std::shared_ptr<AstDataType> data_type) : AstStatement(V_AstType::VarDec) {
        this->name = name;
        this->data_type = data_type;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string name = "";
    std::shared_ptr<AstExpression> size = nullptr;
    std::shared_ptr<AstDataType> data_type;
    
    // Language-specific attributes
    std::string class_name = "";
};

// Represents a structure declaration
struct AstStructDec : AstStatement {
    explicit AstStructDec(std::string var_name, std::string struct_name) : AstStatement(V_AstType::StructDec) {
        this->var_name = var_name;
        this->struct_name = struct_name;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string var_name = "";
    std::string struct_name = "";
    bool no_init = false;
};

// Represents a conditional statement
struct AstIfStmt : AstStatement {
    explicit AstIfStmt() : AstStatement(V_AstType::If) {}
    
    void print(int indent);
    std::string dot(std::string parent) override;
    
    std::shared_ptr<AstBlock> true_block = nullptr;
    std::shared_ptr<AstBlock> false_block = nullptr;
};

// Represents a while statement
struct AstWhileStmt : AstStatement {
    explicit AstWhileStmt() : AstStatement(V_AstType::While) {}
    
    void print(int indent = 0);
    std::string dot(std::string parent) override;

    std::shared_ptr<AstBlock> block = nullptr;
};

// Represents a break statement for a loop
struct AstBreak : AstStatement {
    explicit AstBreak() : AstStatement(V_AstType::Break) {}
    void print();
    std::string dot(std::string parent) override;
};

// Represents a continue statement for a loop
struct AstContinue : AstStatement {
    explicit AstContinue() : AstStatement(V_AstType::Continue) {}
    void print();
    std::string dot(std::string parent) override;
};

//
// Represents an AST tree
//
struct AstTree {
    explicit AstTree(std::string file);
    ~AstTree();
    bool hasStruct(std::string name);
    
    void addGlobalStatement(std::shared_ptr<AstStatement> stmt) {
        block->addStatement(stmt);
    }
    
    void addStruct(std::shared_ptr<AstStruct> s) {
        structs.push_back(s);
    }
    
    void print();
    void dot();
    
    std::string file = "";
    std::shared_ptr<AstBlock> block;
    std::vector<std::shared_ptr<AstStruct>> structs;
};

