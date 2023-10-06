//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <vector>

#include <parser/Parser.hpp>
#include <ast.hpp>

// Builds a variable declaration
// A variable declaration is composed of an Alloca and optionally, an assignment
bool Parser::buildVariableDec(AstBlock *block) {
    Token token = scanner->getNext();
    std::vector<std::string> toDeclare;
    toDeclare.push_back(token.id_val);
    
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected variable name.");
        return false;
    }
    
    token = scanner->getNext();
    
    while (token.type != Colon) {
        if (token.type == Comma) {
            token = scanner->getNext();
            
            if (token.type != Id) {
                syntax->addError(scanner->getLine(), "Expected variable name.");
                return false;
            }
            
            toDeclare.push_back(token.id_val);
        } else if (token.type != Colon) {
            syntax->addError(scanner->getLine(), "Invalid token in variable declaration.");
            return false;
        }
        
        token = scanner->getNext();
    }
    
    token = scanner->getNext();
    DataType dataType = DataType::Void;
    bool isString = false;
    
    switch (token.type) {
        case Bool: dataType = DataType::Bool; break;
        case Char: dataType = DataType::Char; break;
        case Byte: dataType = DataType::Byte; break;
        case UByte: dataType = DataType::UByte; break;
        case Short: dataType = DataType::Short; break;
        case UShort: dataType = DataType::UShort; break;
        case Int: dataType = DataType::Int32; break;
        case UInt: dataType = DataType::UInt32; break;
        case Int64: dataType = DataType::Int64; break;
        case UInt64: dataType = DataType::UInt64; break;
        case Str: dataType = DataType::String; break;
        case Float: dataType = DataType::Float; break;
        case Double: dataType = DataType::Double; break;
        
        case Id: {
            if (enums.find(token.id_val) != enums.end()) {
                EnumDec dec = enums[token.id_val];
                dataType = dec.type;
                break;
            }
        } break;
        
        default: {}
    }
    
    token = scanner->getNext();
    
    // We have an array
    if (token.type == LBracket) {
        AstVarDec *empty = new AstVarDec("", DataType::Array);
        if (!buildExpression(empty, DataType::Int32, RBracket)) return false;   
        
        token = scanner->getNext();
        if (token.type != SemiColon) {
            syntax->addError(scanner->getLine(), "Error: Expected \';\'.");
            return false;
        }
        
        for (std::string name : toDeclare) {
            AstVarDec *vd = new AstVarDec(name, DataType::Array);
            block->addStatement(vd);
            vd->addExpression(empty->getExpression());
            vd->setPtrType(dataType);
            
            // Create an assignment to a malloc call
            AstVarAssign *va = new AstVarAssign(name);
            va->setDataType(DataType::Array);
            va->setPtrType(dataType);
            block->addStatement(va);
            
            AstFuncCallExpr *callMalloc = new AstFuncCallExpr("malloc");
            callMalloc->setArguments(vd->getExpressions());
            va->addExpression(callMalloc);
            
            // In order to get a proper malloc, we need to multiply the argument by
            // the size of the type. Get the arguments, and do that
            AstExpression *arg = callMalloc->getArguments().at(0);
            callMalloc->clearArguments();
            
            AstInt *size;
            if (dataType == DataType::Int32) size = new AstInt(4);
            else if (dataType == DataType::String) size = new AstInt(8);
            else size = new AstInt(1);
            
            AstMulOp *op = new AstMulOp;
            op->setLVal(size);
            op->setRVal(arg);
            callMalloc->addArgument(op);
            
            // Finally, set the size of the declaration
            vd->setPtrSize(arg);
            
            typeMap[name] = std::pair<DataType, DataType>(DataType::Array, dataType);
        }
    
    // We're at the end of the declaration
    } else if (token.type == SemiColon) {
        syntax->addError(scanner->getLine(), "Expected init expression.");
        return false;
        
    // Otherwise, we have a regular variable
    } else {
        AstVarAssign *empty = new AstVarAssign("");
        if (!buildExpression(empty, dataType)) return false;
    
        for (std::string name : toDeclare) {
            AstVarDec *vd = new AstVarDec(name, dataType);
            block->addStatement(vd);
            
            auto typePair = std::pair<DataType, DataType>(dataType, DataType::Void);
            typeMap[name] = typePair;
    
            AstVarAssign *va = new AstVarAssign(name);
            va->setDataType(dataType);
            va->addExpression(empty->getExpression());
            block->addStatement(va);
        }
    }
    
    return true;
}

// Builds a variable assignment
bool Parser::buildVariableAssign(AstBlock *block, Token idToken) {
    DataType dataType = typeMap[idToken.id_val].first;
    AstVarAssign *va = new AstVarAssign(idToken.id_val);
    va->setDataType(dataType);
    block->addStatement(va);
    
    if (!buildExpression(va, dataType)) return false;
    
    if (va->getExpressionCount() == 0) {
        syntax->addError(scanner->getLine(), "Invalid variable assignment.");
        return false;
    }
    
    return true;
}

// Builds an array assignment
bool Parser::buildArrayAssign(AstBlock *block, Token idToken) {
    DataType dataType = typeMap[idToken.id_val].second;
    AstArrayAssign *pa = new AstArrayAssign(idToken.id_val);
    pa->setDataType(typeMap[idToken.id_val].first);
    pa->setPtrType(dataType);
    block->addStatement(pa);
    
    if (!buildExpression(pa, DataType::Int32, RBracket)) return false;
    
    Token token = scanner->getNext();
    if (token.type != Assign) {
        syntax->addError(scanner->getLine(), "Expected \'=\' after array assignment.");
        return false;
    }
    
    if (!buildExpression(pa, dataType)) return false;

    return true;
}

// Builds a constant variable
bool Parser::buildConst(bool isGlobal) {
    Token token = scanner->getNext();
    std::string name = token.id_val;
    
    // Make sure we have a name for our constant
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected constant name.");
        return false;
    }
    
    // Syntax check
    token = scanner->getNext();
    if (token.type != Colon) {
        syntax->addError(scanner->getLine(), "Expected \':\' in constant expression.");
        return false;
    }
    
    // Get the data type
    token = scanner->getNext();
    DataType dataType = DataType::Void;
    
    switch (token.type) {
        case Bool: dataType = DataType::Bool; break;
        case Char: dataType = DataType::Char; break;
        case Byte: dataType = DataType::Byte; break;
        case UByte: dataType = DataType::UByte; break;
        case Short: dataType = DataType::Short; break;
        case UShort: dataType = DataType::UShort; break;
        case Int: dataType = DataType::Int32; break;
        case UInt: dataType = DataType::UInt32; break;
        case Int64: dataType = DataType::Int64; break;
        case UInt64: dataType = DataType::UInt64; break;
        case Str: dataType = DataType::String; break;
        
        default: {
            syntax->addError(scanner->getLine(), "Unknown data type.");
            return false;
        }
    }
    
    // Final syntax check
    token = scanner->getNext();
    if (token.type != Assign) {
        syntax->addError(scanner->getLine(), "Expected \'=\' after const assignment.");
        return false;
    }
    
    // Build the expression. We create a dummy statement for this
    AstExpression *expr = nullptr;
    if (!buildExpression(nullptr, dataType, SemiColon, EmptyToken, &expr, true)) return false;
    
    // Put it all together
    if (isGlobal) {
        globalConsts[name] = std::pair<DataType, AstExpression*>(dataType, expr);
    } else {
        localConsts[name] = std::pair<DataType, AstExpression*>(dataType, expr);
    }
    
    return true;
}
