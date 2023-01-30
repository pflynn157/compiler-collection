//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include "Compiler.hpp"
#include <ast/ast_builder.hpp>

// Compiles a structure declaration
void Compiler::compileStructDeclaration(AstStatement *stmt) {
    AstStructDec *sd = static_cast<AstStructDec *>(stmt);
    StructType *type1 = structTable[sd->getStructName()];
    PointerType *type = PointerType::getUnqual(type1);
    
    AllocaInst *var = builder->CreateAlloca(type);
    symtable[sd->getVarName()] = var;
    typeTable[sd->getVarName()] = AstBuilder::buildStructType(sd->getStructName());
    structVarTable[sd->getVarName()] = sd->getStructName();
    
    // Find the corresponding AST structure
    AstStruct *str = nullptr;
    for (AstStruct *s : tree->getStructs()) {
        if (s->getName() == sd->getStructName()) {
            str = s;
            break;
        }
    }
    if (str == nullptr) return;
    
    // Create a malloc call
    std::vector<Value *> args;
    args.push_back(builder->getInt32(str->getSize()));
    
    Function *callee = mod->getFunction("malloc");
    if (!callee) std::cerr << "Unable to allocate structure." << std::endl;
    Value *ptr = builder->CreateCall(callee, args);
    builder->CreateStore(ptr, var);
    
    // Init the elements
    if (!sd->isNoInit()) {
        int index = 0;
        ptr = builder->CreateLoad(type, var);
        
        for (Var member : str->getItems()) {
            AstExpression *defaultExpr = str->getDefaultExpression(member.name);
            Value *defaultVal = compileValue(defaultExpr);
            
            Value *ep = builder->CreateStructGEP(type1, ptr, index);
            builder->CreateStore(defaultVal, ep);
            
            ++index;
       }
    }
}

// Compiles a structure access expression
Value *Compiler::compileStructAccess(AstExpression *expr, bool isAssign) {
    AstStructAccess *sa = static_cast<AstStructAccess *>(expr);
    Value *ptr = symtable[sa->getName()];
    int pos = getStructIndex(sa->getName(), sa->getMember());
    
    std::string strTypeName = structVarTable[sa->getName()];
    StructType *strType = structTable[strTypeName];
    Type *elementType = structElementTypeTable[strTypeName][pos];
    
    // Load the structure pointer
    PointerType *strTypePtr = PointerType::getUnqual(strType);
    ptr = builder->CreateLoad(strTypePtr, ptr);
    
    // Now, load the structure element
    Value *ep = builder->CreateStructGEP(strType, ptr, pos);
    if (isAssign) return ep;
    else return builder->CreateLoad(elementType, ep);
}

