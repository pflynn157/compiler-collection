//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

// Builds a variable declaration
// A variable declaration is composed of an Alloca and optionally, an assignment
bool Parser::buildVariableDec(std::shared_ptr<AstBlock> block) {
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
    std::shared_ptr<AstDataType> dataType;
    bool isString = false;
    std::string className = "";
    
    switch (token.type) {
        case Bool: dataType = AstBuilder::buildBoolType(); break;
        case Char: dataType = AstBuilder::buildCharType(); break;
        case Byte: dataType = AstBuilder::buildInt8Type(); break;
        case UByte: dataType = AstBuilder::buildInt8Type(true); break;
        case Short: dataType = AstBuilder::buildInt16Type(); break;
        case UShort: dataType = AstBuilder::buildInt16Type(true); break;
        case Int: dataType = AstBuilder::buildInt32Type(); break;
        case UInt: dataType = AstBuilder::buildInt32Type(true); break;
        case Int64: dataType = AstBuilder::buildInt64Type(); break;
        case UInt64: dataType = AstBuilder::buildInt64Type(true); break;
        case Str: dataType = AstBuilder::buildStringType(); break;
        
        default: {
            dataType = AstBuilder::buildObjectType(token.id_val);
            className = token.id_val;
        }
    }
    
    token = scanner->getNext();
    
    // We have a class
    if (token.type == SemiColon ) {
        if (dataType->type != V_AstType::Object) {
            syntax->addError(scanner->getLine(), "Non-objects must have an init expression.");
            return false;
        }
        
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            vd->class_name = className;
            block->addStatement(vd);
            
            block->symbolTable[name] = dataType;
        }
    
    // We have an array
    } else if (token.type == LBracket) {
        /*AstVarDec *empty = new AstVarDec("", DataType::Array);
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
        }*/
        
    // Otherwise, we have a regular variable
    } else {
        std::shared_ptr<AstExprStatement> empty = std::make_shared<AstExprStatement>();
        empty->expression = buildExpression(block, dataType);
        if (!empty->expression) return false;
    
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            block->addStatement(vd);
            block->symbolTable[name] = dataType;
    
            std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
            va->dataType = dataType;
            va->expression = empty->expression;
            block->addStatement(va);
        }
    }
    
    return true;
}

// Builds a variable assignment
bool Parser::buildVariableAssign(std::shared_ptr<AstBlock> block, Token idToken) {
    std::shared_ptr<AstDataType> dataType = block->symbolTable[idToken.id_val];
    std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
    va->dataType = dataType;
    block->addStatement(va);
    
    va->expression = buildExpression(block, dataType);
    if (!va->expression) return false;
    
    if (!va->hasExpression()) {
        syntax->addError(scanner->getLine(), "Invalid variable assignment.");
        return false;
    }
    
    return true;
}

// Builds an array assignment
bool Parser::buildArrayAssign(std::shared_ptr<AstBlock> block, Token idToken) {
    /*DataType dataType = typeMap[idToken.id_val].second;
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
*/
    return true;
}

// Builds a constant variable
bool Parser::buildConst(std::shared_ptr<AstBlock> block, bool isGlobal) {
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
    std::shared_ptr<AstDataType> dataType;
    
    switch (token.type) {
        case Bool: dataType = AstBuilder::buildBoolType(); break;
        case Char: dataType = AstBuilder::buildCharType(); break;
        case Byte: dataType = AstBuilder::buildInt8Type(); break;
        case UByte: dataType = AstBuilder::buildInt8Type(true); break;
        case Short: dataType = AstBuilder::buildInt16Type(); break;
        case UShort: dataType = AstBuilder::buildInt16Type(true); break;
        case Int: dataType = AstBuilder::buildInt32Type(); break;
        case UInt: dataType = AstBuilder::buildInt32Type(true); break;
        case Int64: dataType = AstBuilder::buildInt64Type(); break;
        case UInt64: dataType = AstBuilder::buildInt64Type(true); break;
        case Str: dataType = AstBuilder::buildStringType(); break;
        
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
    std::shared_ptr<AstExpression> expr = buildExpression(block, dataType, SemiColon, EmptyToken, true);
    if (expr == nullptr) return false;
    
    // Put it all together
    if (isGlobal) {
        block->globalConsts[name] = std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>(dataType, expr);
    } else {
        block->localConsts[name] = std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>(dataType, expr);
    }
    
    return true;
}
