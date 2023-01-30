//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

using namespace llvm;

#include <string>
#include <map>
#include <stack>

#include <ast/ast.hpp>

struct CFlags {
    std::string name;
};

class Compiler {
public:
    explicit Compiler(AstTree *tree, CFlags flags);
    void compile();
    void debug();
    void emitLLVM(std::string path);
    void writeAssembly();
    void assemble();
    void link();
protected:
    void compileStatement(AstStatement *stmt);
    Value *compileValue(AstExpression *expr, bool isAssign = false);
    Type *translateType(AstDataType *dataType);
    int getStructIndex(std::string name, std::string member);

    // Function.cpp
    void compileFunction(AstGlobalStatement *global);
    void compileExternFunction(AstGlobalStatement *global);
    void compileFuncCallStatement(AstStatement *stmt);
    void compileReturnStatement(AstStatement *stmt);
    
    // Flow.cpp
    void compileIfStatement(AstStatement *stmt);
    void compileWhileStatement(AstStatement *stmt);
    
    // Variable.cpp
    void compileStructDeclaration(AstStatement *stmt);
    Value *compileStructAccess(AstExpression *expr, bool isAssign = false);
private:
    AstTree *tree;
    CFlags cflags;

    // LLVM stuff
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<Module> mod;
    std::unique_ptr<IRBuilder<>> builder;
    Function *currentFunc;
    AstDataType *currentFuncType;
    
    // The user-defined structure table
    std::map<std::string, StructType*> structTable;
    std::map<std::string, std::string> structVarTable;
    std::map<std::string, std::vector<Type *>> structElementTypeTable;
    
    // Symbol table
    std::map<std::string, AllocaInst *> symtable;
    std::map<std::string, AstDataType *> typeTable;
    
    // Block stack
    int blockCount = 0;
    std::stack<BasicBlock *> breakStack;
    std::stack<BasicBlock *> continueStack;
    std::stack<BasicBlock *> logicalAndStack;
    std::stack<BasicBlock *> logicalOrStack;
};

