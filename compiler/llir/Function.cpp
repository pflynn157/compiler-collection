//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>

#include "Compiler.hpp"
#include <llir.hpp>

//
// Compiles a function and its body
//
void Compiler::compileFunction(std::shared_ptr<AstStatement> global) {
    symtable.clear();
    typeTable.clear();
    structVarTable.clear();
    structArgs.clear();
    
    auto astFunc = std::static_pointer_cast<AstFunction>(global);

    std::vector<Var> astVarArgs = astFunc->args;
    LLIR::Type *funcType = translateType(astFunc->data_type);
    currentFuncType = astFunc->data_type;
    if (currentFuncType->type == V_AstType::Struct)
        funcTypeStruct = std::static_pointer_cast<AstStructType>(astFunc->data_type)->name;
    
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
    
    LLIR::Function *func = LLIR::Function::Create(astFunc->name, LLIR::Linkage::Global, funcType);
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
            if (var.type->type == V_AstType::Struct) {
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

    for (auto stmt : astFunc->block->getBlock()) {
        compileStatement(stmt);
    }
}

//
// Compiles an extern function declaration
//
void Compiler::compileExternFunction(std::shared_ptr<AstStatement> global) {
    auto astFunc = std::static_pointer_cast<AstExternFunction>(global);
    
    std::vector<Var> astVarArgs = astFunc->args;
    LLIR::Type *funcType = translateType(astFunc->data_type);
    
    std::vector<LLIR::Type *> args;
    if (astVarArgs.size() > 0) {
        for (auto var : astVarArgs) {
            LLIR::Type *type = translateType(var.type);
            args.push_back(type);
        }
    }
    
    LLIR::Function *func = LLIR::Function::Create(astFunc->name, LLIR::Linkage::Extern, funcType);
    func->setArgs(args);
    mod->addFunction(func);
}

//
// Compiles a function call statement
// This is different from an expression; this is where its a free-standing statement
//
void Compiler::compileFuncCallStatement(std::shared_ptr<AstStatement> stmt) {
    auto fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
    std::vector<LLIR::Operand *> args;
    
    auto list = std::static_pointer_cast<AstExprList>(fc->expression);
    for (auto arg : list->list) {
        LLIR::Operand *val = compileValue(arg);
        args.push_back(val);
    }
    
    builder->createVoidCall(fc->name, args);
}

//
// Compiles a return statement
// TODO: We may want to rethink this some
//
void Compiler::compileReturnStatement(std::shared_ptr<AstStatement> stmt) {
    if (stmt->hasExpression()) {
        LLIR::Operand *op = compileValue(stmt->expression, currentFuncType);
        LLIR::Type *type = translateType(currentFuncType);
        if (currentFuncType->type == V_AstType::Struct) {
            LLIR::StructType *type = structTable[funcTypeStruct];
            builder->createRet(type, op);
        } else {
            builder->createRet(type, op);
        }
    } else {
        builder->createRetVoid();
    }
}

