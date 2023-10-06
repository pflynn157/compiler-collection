//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <LLVM/Compiler.hpp>

//
// Compiles a function and its body
//
void Compiler::compileFunction(AstGlobalStatement *global) {
    symtable.clear();
    typeTable.clear();
    ptrTable.clear();
    structVarTable.clear();
    
    AstFunction *astFunc = static_cast<AstFunction *>(global);

    std::vector<Var> astVarArgs = astFunc->getArguments();
    FunctionType *FT;
    Type *funcType = translateType(astFunc->getDataType(), astFunc->getPtrType(), astFunc->getDataTypeName());
    //if (astFunc->getDataType() == DataType::Struct) {
    //    funcType = PointerType::getUnqual(funcType);
    //}
    currentFuncType = astFunc->getDataType();
    
    if (astVarArgs.size() == 0) {
        FT = FunctionType::get(funcType, false);
    } else {
        std::vector<Type *> args;
        for (auto var : astVarArgs) {
            Type *type = translateType(var.type, var.subType, var.typeName);
            if (var.type == DataType::Struct) {
                type = PointerType::getUnqual(type);
            }
            args.push_back(type);
        }
        
        FT = FunctionType::get(funcType, args, false);
    }
    
    Function *func = Function::Create(FT, Function::ExternalLinkage, astFunc->getName(), mod.get());
    currentFunc = func;
    
    if (cflags.nvptx) {
        func->setCallingConv(CallingConv::PTX_Kernel);
    }

    BasicBlock *mainBlock = BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(mainBlock);
    
    // Load and store any arguments
    if (astVarArgs.size() > 0) {
        for (int i = 0; i<astVarArgs.size(); i++) {
            Var var = astVarArgs.at(i);
            
            // Build the alloca for the local var
            Type *type = translateType(var.type, var.subType, var.typeName);
            if (var.type == DataType::Struct) {
                symtable[var.name] = (AllocaInst *)func->getArg(i);
                typeTable[var.name] = var.type;
                ptrTable[var.name] = var.subType;
                structVarTable[var.name] = var.typeName;
                continue;
            }
            
            AllocaInst *alloca = builder->CreateAlloca(type);
            symtable[var.name] = alloca;
            typeTable[var.name] = var.type;
            ptrTable[var.name] = var.subType;
            
            // Store the variable
            Value *param = func->getArg(i);
            builder->CreateStore(param, alloca);
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
    FunctionType *FT;
    
    Type *retType = translateType(astFunc->getDataType());
    
    if (astVarArgs.size() == 0) {
        FT = FunctionType::get(retType, false);
    } else {
        std::vector<Type *> args;
        for (auto var : astVarArgs) {
            Type *type = translateType(var.type, var.subType);
            args.push_back(type);
        }
        
        FT = FunctionType::get(retType, args, false);
    }
    
    Function::Create(FT, Function::ExternalLinkage, astFunc->getName(), mod.get());
}

//
// Compiles a function call statement
// This is different from an expression; this is where its a free-standing statement
//
// // TODO: We should not do error handeling in the compiler. Check for invalid functions in the AST level
//
void Compiler::compileFuncCallStatement(AstStatement *stmt) {
    AstFuncCallStmt *fc = static_cast<AstFuncCallStmt *>(stmt);
    std::vector<Value *> args;
    
    for (auto stmt : stmt->getExpressions()) {
        Value *val = compileValue(stmt);
        args.push_back(val);
    }
    
    Function *callee = mod->getFunction(fc->getName());
    if (!callee) std::cerr << "Invalid function call statement." << std::endl;
    builder->CreateCall(callee, args);
}

//
// Compiles a return statement
// TODO: We may want to rethink this some
//
void Compiler::compileReturnStatement(AstStatement *stmt) {
    if (stmt->getExpressionCount() == 0) {
        builder->CreateRetVoid();
    } else if (stmt->getExpressionCount() == 1) {
        Value *val = compileValue(stmt->getExpressions().at(0), currentFuncType);
        if (currentFuncType == DataType::Struct) {
            Value *ld = builder->CreateLoad(val);
            builder->CreateRet(ld);
        } else {
            builder->CreateRet(val);
        }
    } else {
        builder->CreateRetVoid();
    }
}

