//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <memory>

#include "Compiler.hpp"
#include <ast/ast_builder.hpp>

// Compiles a structure declaration
void Compiler::compileStructDeclaration(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstStructDec> sd = std::static_pointer_cast<AstStructDec>(stmt);
    StructType *type1 = structTable[sd->struct_name];
    PointerType *type = PointerType::getUnqual(type1);
    
    AllocaInst *var = builder->CreateAlloca(type);
    symtable[sd->var_name] = var;
    typeTable[sd->var_name] = AstBuilder::buildStructType(sd->struct_name);
    structVarTable[sd->var_name] = sd->struct_name;
    
    // Find the corresponding AST structure
    std::shared_ptr<AstStruct> str = nullptr;
    for (auto const &s : tree->structs) {
        if (s->name == sd->struct_name) {
            str = s;
            break;
        }
    }
    if (str == nullptr) return;
    
    // Create a malloc call
    std::vector<Value *> args;
    args.push_back(builder->getInt32(str->size));
    
    Function *callee = mod->getFunction("malloc");
    if (!callee) std::cerr << "Unable to allocate structure." << std::endl;
    Value *ptr = builder->CreateCall(callee, args);
    builder->CreateStore(ptr, var);
    
    // Init the elements
    if (!sd->no_init) {
        int index = 0;
        ptr = builder->CreateLoad(type, var);
        
        for (Var member : str->items) {
            std::shared_ptr<AstExpression> defaultExpr = str->default_expressions[member.name];
            Value *defaultVal = compileValue(defaultExpr);
            
            Value *ep = builder->CreateStructGEP(type1, ptr, index);
            builder->CreateStore(defaultVal, ep);
            
            ++index;
       }
    }
}

// Compiles a structure access expression
Value *Compiler::compileStructAccess(std::shared_ptr<AstExpression> expr, bool isAssign) {
    std::shared_ptr<AstStructAccess> sa = std::static_pointer_cast<AstStructAccess>(expr);
    Value *ptr = symtable[sa->var];
    int pos = getStructIndex(sa->var, sa->member);
    
    std::string strTypeName = structVarTable[sa->var];
    StructType *strType = structTable[strTypeName];
    Type *elementType = structElementTypeTable[strTypeName][pos];
    
    // Load the structure pointer
    PointerType *strTypePtr = PointerType::getUnqual(strType);
    ptr = builder->CreateLoad(strTypePtr, ptr);
    
    // Now, load the structure element
    Value *ep = builder->CreateStructGEP(strType, ptr, pos);
    if (sa->access_expression) {
        Type *baseElementType;
        if (strTypeName == "__int8_array") baseElementType = Type::getInt8Ty(*context);
        else if (strTypeName == "__int16_array") baseElementType = Type::getInt16Ty(*context);
        else if (strTypeName == "__int64_array") baseElementType = Type::getInt64Ty(*context);
        else baseElementType = Type::getInt32Ty(*context);
    
        Value *idx = compileValue(sa->access_expression);
        Value *ep_load = builder->CreateLoad(elementType, ep);
        Value *ep_idx = builder->CreateGEP(baseElementType, ep_load, idx);
        if (isAssign) return ep_idx;
        else return builder->CreateLoad(baseElementType, ep_idx);
    } else {
        if (isAssign) return ep;
        else return builder->CreateLoad(elementType, ep);
    }
}

