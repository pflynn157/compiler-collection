//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include "Compiler.hpp"

//
// Compiles a function and its body
//
void Compiler::compileFunction(std::shared_ptr<AstStatement> global) {
    symtable.clear();
    typeTable.clear();
    structVarTable.clear();
    
    std::shared_ptr<AstFunction> astFunc = std::static_pointer_cast<AstFunction>(global);

    std::vector<Var> astVarArgs = astFunc->args;
    FunctionType *FT;
    Type *funcType = translateType(astFunc->data_type);
    currentFuncType = astFunc->data_type;
    
    if (astVarArgs.size() == 0) {
        FT = FunctionType::get(funcType, false);
    } else {
        std::vector<Type *> args;
        for (auto var : astVarArgs) {
            Type *type = translateType(var.type);
            if (var.type->type == V_AstType::Struct) {
                type = PointerType::getUnqual(type);
            }
            args.push_back(type);
        }
        
        FT = FunctionType::get(funcType, args, false);
    }
    
    Function *func = Function::Create(FT, Function::ExternalLinkage, astFunc->name, mod.get());
    currentFunc = func;

    BasicBlock *mainBlock = BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(mainBlock);
    
    // Load and store any arguments
    if (astVarArgs.size() > 0) {
        for (int i = 0; i<astVarArgs.size(); i++) {
            Var var = astVarArgs.at(i);
            
            // Build the alloca for the local var
            Type *type = translateType(var.type);
            if (var.type->type == V_AstType::Struct) {
                symtable[var.name] = (AllocaInst *)func->getArg(i);
                typeTable[var.name] = var.type;
                structVarTable[var.name] = std::static_pointer_cast<AstStructType>(var.type)->name;
                continue;
            }
            
            AllocaInst *alloca = builder->CreateAlloca(type);
            symtable[var.name] = alloca;
            typeTable[var.name] = var.type;
            
            // Store the variable
            Value *param = func->getArg(i);
            builder->CreateStore(param, alloca);
        }
    }

    for (auto stmt : astFunc->block->getBlock()) {
        compileStatement(stmt);
    }
}

//
// Compiles an extern function declaration
//
void Compiler::compileExternFunction(std::shared_ptr<AstStatement> global) {
    std::shared_ptr<AstExternFunction> astFunc = std::static_pointer_cast<AstExternFunction>(global);
    
    std::vector<Var> astVarArgs = astFunc->args;
    FunctionType *FT;
    
    Type *retType = translateType(astFunc->data_type);
    
    if (astVarArgs.size() == 0) {
        FT = FunctionType::get(retType, astFunc->varargs);
    } else {
        std::vector<Type *> args;
        for (auto var : astVarArgs) {
            Type *type = translateType(var.type);
            args.push_back(type);
        }
        
        FT = FunctionType::get(retType, args, astFunc->varargs);
    }
    
    Function::Create(FT, Function::ExternalLinkage, astFunc->name, mod.get());
}

//
// Compiles a function call statement
// This is different from an expression; this is where its a free-standing statement
//
// // TODO: We should not do error handeling in the compiler. Check for invalid functions in the AST level
//
void Compiler::compileFuncCallStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstFuncCallStmt> fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
    std::vector<Value *> args;
    
    /*for (auto stmt : stmt->getExpressions()) {
        Value *val = compileValue(stmt);
        args.push_back(val);
    }*/
    std::shared_ptr<AstExprList> list = std::static_pointer_cast<AstExprList>(fc->expression);
    for (auto arg : list->list) {
        Value *val = compileValue(arg);
        args.push_back(val);
    }
    
    Function *callee = mod->getFunction(fc->name);
    if (!callee) std::cerr << "Invalid function call statement." << std::endl;
    builder->CreateCall(callee, args);
}

//
// Compiles a return statement
// TODO: We may want to rethink this some
//
void Compiler::compileReturnStatement(std::shared_ptr<AstStatement> stmt) {
    if (!stmt->hasExpression()) {
        builder->CreateRetVoid();
    } else if (stmt->hasExpression()) {
        Value *val = compileValue(stmt->expression);
        if (currentFuncType->type == V_AstType::Struct) {
            std::shared_ptr<AstStructType> sType = std::static_pointer_cast<AstStructType>(currentFuncType);
            StructType *type = structTable[sType->name];
            Value *ld = builder->CreateLoad(type, val);
            builder->CreateRet(ld);
        } else {
            builder->CreateRet(val);
        }
    } else {
        builder->CreateRetVoid();
    }
}

