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
#include <memory>

#include <ast/ast.hpp>

struct CFlags {
    std::string name;
};

class Compiler {
public:
    explicit Compiler(std::shared_ptr<AstTree> tree, CFlags flags);
    void compile();
    void debug();
    void emitLLVM(std::string path);
    void writeAssembly();
protected:
    void compileStatement(std::shared_ptr<AstStatement> stmt);
    Value *compileValue(std::shared_ptr<AstExpression> expr, bool isAssign = false);
    Type *translateType(std::shared_ptr<AstDataType> dataType);
    int getStructIndex(std::string name, std::string member);

    // Function.cpp
    void compileFunction(std::shared_ptr<AstStatement> global);
    void compileExternFunction(std::shared_ptr<AstStatement> global);
    void compileFuncCallStatement(std::shared_ptr<AstStatement> stmt);
    void compileReturnStatement(std::shared_ptr<AstStatement> stmt);
    
    // Flow.cpp
    void compileIfStatement(std::shared_ptr<AstStatement> stmt);
    void compileWhileStatement(std::shared_ptr<AstStatement> stmt);
    void compileForStatement(std::shared_ptr<AstStatement> stmt);
    
    // Variable.cpp
    void compileStructDeclaration(std::shared_ptr<AstStatement> stmt);
    Value *compileStructAccess(std::shared_ptr<AstExpression> expr, bool isAssign = false);
private:
    std::shared_ptr<AstTree> tree;
    CFlags cflags;

    // LLVM stuff
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<Module> mod;
    std::unique_ptr<IRBuilder<>> builder;
    Function *currentFunc;
    std::shared_ptr<AstDataType> currentFuncType;
    
    // The user-defined structure table
    std::map<std::string, StructType*> structTable;
    std::map<std::string, std::string> structVarTable;
    std::map<std::string, std::vector<Type *>> structElementTypeTable;
    
    // Symbol table
    std::map<std::string, AllocaInst *> symtable;
    std::map<std::string, std::shared_ptr<AstDataType>> typeTable;
    
    // Block stack
    int blockCount = 0;
    std::stack<BasicBlock *> breakStack;
    std::stack<BasicBlock *> continueStack;
    std::stack<BasicBlock *> logicalAndStack;
    std::stack<BasicBlock *> logicalOrStack;
};

