//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <stack>

#include <llir/llir.hpp>
#include <llir/irbuilder.hpp>

#include <ast/ast.hpp>

struct CFlags {
    std::string name;
};

class Compiler {
public:
    explicit Compiler(AstTree *tree, CFlags flags);
    void compile();
    void debug();
    void writeAssembly(bool printTransform = false);
    void assemble();
    void link();
protected:
    void compileStatement(AstStatement *stmt);
    LLIR::Operand *compileValue(AstExpression *expr, AstDataType *dataType = nullptr, LLIR::Block *destBlock = nullptr, bool isAssign = false);
    LLIR::Type *translateType(AstDataType *dataType);
    int getStructIndex(std::string name, std::string member);

    // Function.cpp
    void compileFunction(AstGlobalStatement *global);
    void compileExternFunction(AstGlobalStatement *global);
    void compileFuncCallStatement(AstStatement *stmt);
    void compileReturnStatement(AstStatement *stmt);
    
    // Flow.cpp
    void compileIfStatement(AstStatement *stmt);
    void compileWhileStatement(AstStatement *stmt);
private:
    AstTree *tree;
    CFlags cflags;

    // LLIR stuff
    LLIR::Module *mod;
    LLIR::IRBuilder *builder;
    LLIR::Function *currentFunc;
    AstDataType *currentFuncType;
    std::string funcTypeStruct = "";
    
    // The user-defined structure table
    std::map<std::string, LLIR::StructType*> structTable;
    std::map<std::string, std::string> structVarTable;
    std::map<std::string, std::vector<LLIR::Type *>> structElementTypeTable;
    std::vector<std::string> structArgs;
    
    // Symbol table
    std::map<std::string, LLIR::Reg *> symtable;
    std::map<std::string, AstDataType *> typeTable;
    
    // Block stack
    int blockCount = 0;
    std::stack<LLIR::Block *> breakStack;
    std::stack<LLIR::Block *> continueStack;
    std::stack<LLIR::Block *> logicalAndStack;
    std::stack<LLIR::Block *> logicalOrStack;
};

