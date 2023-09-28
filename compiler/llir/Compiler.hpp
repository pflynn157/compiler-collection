//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <stack>
#include <memory>

#include <llir.hpp>
#include <irbuilder.hpp>

#include <ast/ast.hpp>

struct CFlags {
    std::string name;
};

class Compiler {
public:
    explicit Compiler(std::shared_ptr<AstTree> tree, CFlags flags);
    void compile();
    void debug();
    void writeAssembly(bool printTransform = false);
protected:
    void compileStatement(std::shared_ptr<AstStatement> stmt);
    LLIR::Operand *compileValue(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> dataType = nullptr,
                                LLIR::Block *destBlock = nullptr, bool isAssign = false);
    LLIR::Type *translateType(std::shared_ptr<AstDataType> dataType);
    int getStructIndex(std::string name, std::string member);

    // Function.cpp
    void compileFunction(std::shared_ptr<AstStatement> global);
    void compileExternFunction(std::shared_ptr<AstStatement> global);
    void compileFuncCallStatement(std::shared_ptr<AstStatement> stmt);
    void compileReturnStatement(std::shared_ptr<AstStatement> stmt);
    
    // Flow.cpp
    void compileIfStatement(std::shared_ptr<AstStatement> stmt);
    void compileWhileStatement(std::shared_ptr<AstStatement> stmt);
private:
    std::shared_ptr<AstTree> tree;
    CFlags cflags;

    // LLIR stuff
    LLIR::Module *mod;
    LLIR::IRBuilder *builder;
    LLIR::Function *currentFunc;
    std::shared_ptr<AstDataType> currentFuncType;
    std::string funcTypeStruct = "";
    
    // The user-defined structure table
    std::map<std::string, LLIR::StructType*> structTable;
    std::map<std::string, std::string> structVarTable;
    std::map<std::string, std::vector<LLIR::Type *>> structElementTypeTable;
    std::vector<std::string> structArgs;
    
    // Symbol table
    std::map<std::string, LLIR::Reg *> symtable;
    std::map<std::string, std::shared_ptr<AstDataType>> typeTable;
    
    // Block stack
    int blockCount = 0;
    std::stack<LLIR::Block *> breakStack;
    std::stack<LLIR::Block *> continueStack;
    std::stack<LLIR::Block *> logicalAndStack;
    std::stack<LLIR::Block *> logicalOrStack;
};

