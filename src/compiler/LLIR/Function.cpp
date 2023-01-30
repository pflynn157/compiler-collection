//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include "Compiler.hpp"
#include <llir/llir.hpp>

//
// Compiles a function and its body
//
void Compiler::compileFunction(AstGlobalStatement *global) {
    symtable.clear();
    typeTable.clear();
    structVarTable.clear();
    structArgs.clear();
    
    AstFunction *astFunc = static_cast<AstFunction *>(global);

    std::vector<Var> astVarArgs = astFunc->getArguments();
    LLIR::Type *funcType = translateType(astFunc->getDataType());
    currentFuncType = astFunc->getDataType();
    if (currentFuncType->getType() == V_AstType::Struct)
        funcTypeStruct = static_cast<AstStructType *>(astFunc->getDataType())->getName();
    
    std::vector<LLIR::Type *> args;
    if (astVarArgs.size() > 0) {
        for (auto var : astVarArgs) {
            LLIR::Type *type = translateType(var.type);
            //if (var.type == V_AstType::Struct) {
            //    type = translateType(V_AstType::Ptr, var.type, var.typeName);
            //}
            args.push_back(type);
        }
    }
    
    LLIR::Function *func = LLIR::Function::Create(astFunc->getName(), LLIR::Linkage::Global, funcType);
    func->setArgs(args);
    currentFunc = func;
    mod->addFunction(func);
    
    builder->setCurrentFunction(func);
    builder->createBlock("entry");
    
    // Load and store any arguments
    if (astVarArgs.size() > 0) {
        for (int i = 0; i<astVarArgs.size(); i++) {
            Var var = astVarArgs.at(i);
            
            // Build the alloca for the local var
            LLIR::Type *type = args.at(i);
            // TODO: Combine this with below
            if (var.type->getType() == V_AstType::Struct) {
                LLIR::Reg *alloca = builder->createAlloca(type);
                symtable[var.name] = alloca;
                typeTable[var.name] = var.type;
                //structVarTable[var.name] = var.typeName;
                structArgs.push_back(var.name);
                
                LLIR::Operand *param = func->getArg(i);
                builder->createStore(type, param, alloca);
                continue;
            }
            
            LLIR::Reg *alloca = builder->createAlloca(type);
            symtable[var.name] = alloca;
            typeTable[var.name] = var.type;
            
            // Store the variable
            LLIR::Operand *param = func->getArg(i);
            builder->createStore(type, param, alloca);
        }
    }

    for (auto stmt : astFunc->getBlock()->getBlock()) {
        compileStatement(stmt);
    }
}

//
// Compiles an extern function declaration
//
void Compiler::compileExternFunction(AstGlobalStatement *global) {
    AstExternFunction *astFunc = static_cast<AstExternFunction *>(global);
    
    std::vector<Var> astVarArgs = astFunc->getArguments();
    LLIR::Type *funcType = translateType(astFunc->getDataType());
    
    std::vector<LLIR::Type *> args;
    if (astVarArgs.size() > 0) {
        for (auto var : astVarArgs) {
            LLIR::Type *type = translateType(var.type);
            args.push_back(type);
        }
    }
    
    LLIR::Function *func = LLIR::Function::Create(astFunc->getName(), LLIR::Linkage::Extern, funcType);
    func->setArgs(args);
    mod->addFunction(func);
}

//
// Compiles a function call statement
// This is different from an expression; this is where its a free-standing statement
//
void Compiler::compileFuncCallStatement(AstStatement *stmt) {
    AstFuncCallStmt *fc = static_cast<AstFuncCallStmt *>(stmt);
    std::vector<LLIR::Operand *> args;
    
    AstExprList *list = static_cast<AstExprList *>(fc->getExpression());
    for (auto arg : list->getList()) {
        LLIR::Operand *val = compileValue(arg);
        args.push_back(val);
    }
    
    builder->createVoidCall(fc->getName(), args);
}

//
// Compiles a return statement
// TODO: We may want to rethink this some
//
void Compiler::compileReturnStatement(AstStatement *stmt) {
    if (stmt->hasExpression()) {
        LLIR::Operand *op = compileValue(stmt->getExpression(), currentFuncType);
        LLIR::Type *type = translateType(currentFuncType);
        if (currentFuncType->getType() == V_AstType::Struct) {
            LLIR::StructType *type = structTable[funcTypeStruct];
            builder->createRet(type, op);
        } else {
            builder->createRet(type, op);
        }
    } else {
        builder->createRetVoid();
    }
}

