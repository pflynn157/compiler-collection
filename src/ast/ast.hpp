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
    Struct
};

// Forward declarations
class AstExpression;
class AstStatement;

//
// The base of all AST nodes
//
class AstNode {
public:
    explicit AstNode();
    explicit AstNode(V_AstType type);
    
    V_AstType getType();
    
    virtual void print() {}
protected:
    V_AstType type = V_AstType::None;
};

//
// AST data types
//

// The base of all AST data types
class AstDataType : public AstNode {
public:
    explicit AstDataType(V_AstType type);
    explicit AstDataType(V_AstType type, bool _isUnsigned);
    
    void setUnsigned(bool _isUnsigned);
    bool isUnsigned();
    
    void print() override;
protected:
    bool _isUnsigned = false;
};

// Represents a pointer type
class AstPointerType : public AstDataType {
public:
    explicit AstPointerType(std::shared_ptr<AstDataType> baseType);
    std::shared_ptr<AstDataType> getBaseType();
    
    void print() override;
protected:
    std::shared_ptr<AstDataType> baseType = nullptr;
};

// Represents a structure type
class AstStructType : public AstDataType {
public:
    explicit AstStructType(std::string name);
    std::string getName();
    
    void print() override;
protected:
    std::string name = "";
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
class AstStruct : public AstNode {
public:
    explicit AstStruct(std::string name);
    
    void addItem(Var var, std::shared_ptr<AstExpression> defaultExpression);
    
    std::string getName();
    std::vector<Var> getItems();
    int getSize();
    
    std::shared_ptr<AstExpression> getDefaultExpression(std::string name);
    
    void print();
    std::string dot(std::string parent);
private:
    std::string name;
    std::vector<Var> items;
    std::map<std::string, std::shared_ptr<AstExpression>> defaultExpressions;
    int size = 0;
};

//
// Represents an AstBlock
// Blocks hold tables and symbol information
//
class AstBlock : public AstNode {
public:
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
    
    void print(int indent = 4);
    std::string dot(std::string parent);
protected:
    std::vector<std::shared_ptr<AstStatement>> block;
    std::map<std::string, std::shared_ptr<AstDataType>> symbolTable;
    std::vector<std::string> vars;
};

//
// -----------------------------------------------------
// AstExpressions
// -----------------------------------------------------
//

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


//
// -----------------------------------------------------
// AstStatements
// -----------------------------------------------------
//

// Represents an AST statement
class AstStatement : public AstNode {
public:
    explicit AstStatement();
    explicit AstStatement(V_AstType type);
    
    void setExpression(std::shared_ptr<AstExpression> expr);
    std::shared_ptr<AstExpression> getExpression();
    bool hasExpression();
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
private:
    std::shared_ptr<AstExpression> expr = nullptr;
};

// Represents a function, external declaration, or global variable
// TODO: Delete
class AstGlobalStatement : public AstNode {
public:
    explicit AstGlobalStatement() : AstNode(V_AstType::None) {}
    explicit AstGlobalStatement(V_AstType type) : AstNode(type) {}
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
};

// Represents an extern function
// TODO: Rebase
class AstExternFunction : public AstGlobalStatement {
public:
    explicit AstExternFunction(std::string name) : AstGlobalStatement(V_AstType::ExternFunc) {
        this->name = name;
    }
    
    void addArgument(Var arg) { this->args.push_back(arg); }
    void setArguments(std::vector<Var> args) { this->args = args; }
    
    void setDataType(std::shared_ptr<AstDataType> dataType) {
        this->dataType = dataType;
    }
    
    void setVarArgs() { this->varargs = true; }
    bool isVarArgs() { return this->varargs; }
    
    std::string getName() { return name; }
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    std::vector<Var> getArguments() { return args; }
    
    void print() override;
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstDataType> dataType;
    bool varargs = false;
};

// Represents a function
// TODO: Rebase
class AstFunction : public AstGlobalStatement {
public:
    explicit AstFunction(std::string name) : AstGlobalStatement(V_AstType::Func) {
        this->name = name;
        block = std::make_shared<AstBlock>();
    }
    
    std::string getName() { return name; }
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    std::vector<Var> getArguments() { return args; }
    std::shared_ptr<AstBlock> getBlock() { return block; }
    
    void setName(std::string name) { this->name = name; }
    
    void setArguments(std::vector<Var> args) { this->args = args; }
    
    void addStatement(std::shared_ptr<AstStatement> statement) {
        block->addStatement(statement);
    }
    
    void setDataType(std::shared_ptr<AstDataType> dataType) {
        this->dataType = dataType;
    }
    
    void print() override;
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block;
    std::shared_ptr<AstDataType> dataType;
    std::string dtName = "";
};

// Represents an AST expression statement
// This is basically the same as a statement
class AstExprStatement : public AstStatement {
public:
    explicit AstExprStatement() : AstStatement(V_AstType::ExprStmt) {}
    
    void setDataType(std::shared_ptr<AstDataType> dataType) {
        this->dataType = dataType;
    }
    
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::shared_ptr<AstDataType> dataType;
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
    explicit AstVarDec(std::string name, std::shared_ptr<AstDataType> dataType) : AstStatement(V_AstType::VarDec) {
        this->name = name;
        this->dataType = dataType;
    }
    
    void setDataType(std::shared_ptr<AstDataType> dataType) { this->dataType = dataType; }
    void setPtrSize(std::shared_ptr<AstExpression> size) { this->size = size; }
    
    std::string getName() { return name; }
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    std::shared_ptr<AstExpression> getPtrSize() { return size; }
    
    void print();
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::shared_ptr<AstExpression> size = nullptr;
    std::shared_ptr<AstDataType> dataType;
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

//
// Represents an AST tree
//
class AstTree {
public:
    explicit AstTree(std::string file);
    ~AstTree();
    
    std::string getFile();
    
    std::vector<std::shared_ptr<AstGlobalStatement>> getGlobalStatements();
    std::vector<std::shared_ptr<AstStruct>> getStructs();
    
    bool hasStruct(std::string name);
    
    void addGlobalStatement(std::shared_ptr<AstGlobalStatement> stmt);
    void addStruct(std::shared_ptr<AstStruct> s);
    
    void print();
    void dot();
private:
    std::string file = "";
    std::vector<std::shared_ptr<AstGlobalStatement>> global_statements;
    std::vector<std::shared_ptr<AstStruct>> structs;
};

