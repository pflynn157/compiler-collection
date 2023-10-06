//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <parser/Parser.hpp>
#include <ast.hpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::vector<Var> &args) {
    Token token = scanner->getNext();
    if (token.type == LParen) {
        token = scanner->getNext();
        while (token.type != Eof && token.type != RParen) {
            Token t1 = token;
            Token t2 = scanner->getNext();
            Token t3 = scanner->getNext();
            Var v;
            v.subType = DataType::Void;
            
            if (t1.type != Id) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2.type != Colon) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            switch (t3.type) {
                case Bool: v.type = DataType::Bool; break;
                case Char: v.type = DataType::Char; break;
                case Byte: v.type = DataType::Byte; break;
                case UByte: v.type = DataType::UByte; break;
                case Short: v.type = DataType::Short; break;
                case UShort: v.type = DataType::UShort; break;
                case Int: v.type = DataType::Int32; break;
                case UInt: v.type = DataType::UInt32; break;
                case Int64: v.type = DataType::Int64; break;
                case UInt64: v.type = DataType::UInt64; break;
                case Str: v.type = DataType::String; break;
                case Float: v.type = DataType::Float; break;
                case Double: v.type = DataType::Double; break;
                
                case Id: {
                    bool isStruct = false;
                    for (auto s : tree->getStructs()) {
                        if (s->getName() == t3.id_val) {
                            isStruct = true;
                            break;
                        }
                    }
                    
                    if (isStruct) {
                        v.type = DataType::Struct;
                        v.typeName = t3.id_val;
                    }
                } break;
                
                default: {
                    syntax->addError(scanner->getLine(), "Invalid function argument: Unknown type.");
                    return false;
                }
            }
            
            v.name = t1.id_val;
            
            token = scanner->getNext();
            if (token.type == Comma) {
                token = scanner->getNext();
            } else if (token.type == LBracket) {
                Token token1 = scanner->getNext();
                Token token2 = scanner->getNext();
                
                if (token1.type != RBracket) {
                    syntax->addError(scanner->getLine(), "Invalid type syntax.");
                    return false;
                }
                
                if (token2.type == Comma) token = scanner->getNext();
                else token = token2;
                
                v.subType = v.type;
                v.type = DataType::Array;
            }
            
            args.push_back(v);
            typeMap[v.name] = std::pair<DataType, DataType>(v.type, v.subType);
        }
    } else {
        scanner->rewind(token);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(Token startToken, std::string className) {
    typeMap.clear();
    localConsts.clear();
    
    Token token;
    bool isExtern = false;

    // Handle extern function
    if (startToken.type == Extern) {
        isExtern = true;
    }

    // Make sure we have a function name
    token = scanner->getNext();
    std::string funcName = token.id_val;
    
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected function name.");
        return false;
    }
    
    // Get arguments
    std::vector<Var> args;
    if (className != "") {
        Var classV;
        classV.name = "this";
        classV.type = DataType::Struct;
        classV.subType = DataType::Void;
        classV.typeName = className;
        args.push_back(classV);
        
        typeMap["this"] = std::pair<DataType, DataType>(classV.type, classV.subType);
    }
    
    if (!getFunctionArgs(args)) return false;

    // Check to see if there's any return type
    token = scanner->getNext();
    DataType funcType = DataType::Void;
    DataType ptrType = DataType::Void;
    std::string retName = "";
    
    if (token.type == Arrow) {
        token = scanner->getNext();
        switch (token.type) {
            case Int: funcType = DataType::Int32; break;
            case Str: funcType = DataType::String; break;
            
            case Id: {
                if (enums.find(token.id_val) != enums.end()) {
                    EnumDec dec = enums[token.id_val];
                    funcType = dec.type;
                    break;
                }
                
                bool isStruct = false;
                    for (auto s : tree->getStructs()) {
                        if (s->getName() == token.id_val) {
                            isStruct = true;
                            break;
                        }
                    }
                    
                    if (isStruct) {
                        //v.type = DataType::Struct;
                        //v.typeName = token.id_val;
                        funcType = DataType::Struct;
                        retName = token.id_val;
                    }
            } break;
            
            default: {}
        }
    
        token = scanner->getNext();
        if (token.type == LBracket) {
            token = scanner->getNext();
            if (token.type != RBracket) {
                syntax->addError(scanner->getLine(), "Invalid function type.");
                return false;
            }
            
            ptrType = funcType;
            funcType = DataType::Array;
            
            token = scanner->getNext();
        }
    }
    
    // Do syntax error check
    if (token.type == SemiColon && !isExtern) {
        syntax->addError(scanner->getLine(), "Expected \';\' for extern function.");
        return false;
    } else if (token.type == Is && isExtern) {
        syntax->addError(scanner->getLine(), "Expected \'is\' keyword.");
        return false;
    }

    // Create the function object
    if (isExtern) {
        AstExternFunction *ex = new AstExternFunction(funcName);
        ex->setArguments(args);
        ex->setDataType(funcType);
        tree->addGlobalStatement(ex);
        return true;
    }
    
    AstFunction *func = new AstFunction(funcName);
    func->setDataType(funcType, ptrType);
    if (funcType == DataType::Struct) func->setDataTypeName(retName);
    func->setArguments(args);
    
    //if (className == "") tree->addGlobalStatement(func);
    //else currentClass->addFunction(func);
    tree->addGlobalStatement(func);
    if (className != "") {
        std::string fullName = className + "_" + funcName;
        func->setName(fullName);
    }
    
    // Build the body
    int stopLayer = 0;
    if (className != "") {
        stopLayer = 1;
        ++layer;
    }
    
    if (!buildBlock(func->getBlock(), stopLayer)) return false;
    
    // Make sure we end with a return statement
    AstType lastType = func->getBlock()->getBlock().back()->getType();
    if (lastType == AstType::Return) {
        AstStatement *ret = func->getBlock()->getBlock().back();
        if (func->getDataType() == DataType::Void && ret->getExpressionCount() > 0) {
            syntax->addError(scanner->getLine(), "Cannot return from void function.");
            return false;
        } else if (ret->getExpressionCount() == 0) {
            syntax->addError(scanner->getLine(), "Expected return value.");
            return false;
        }
    } else {
        if (func->getDataType() == DataType::Void) {
            func->addStatement(new AstReturnStmt);
        } else {
            syntax->addError(scanner->getLine(), "Expected return statement.");
            return false;
        }
    }
    
    if (className != "") {
        AstFunction *func2 = new AstFunction(funcName);
        func2->setDataType(funcType, ptrType);
        func2->setArguments(args);
        currentClass->addFunction(func2);
        
        AstBlock *block2 = func->getBlock();
        func2->getBlock()->addStatements(block2->getBlock());
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(AstBlock *block, Token idToken) {
    AstFuncCallStmt *fc = new AstFuncCallStmt(idToken.id_val);
    block->addStatement(fc);
    
    if (!buildExpression(fc, DataType::Void, RParen, Comma)) return false;
    
    Token token = scanner->getNext();
    if (token.type != SemiColon) {
        syntax->addError(scanner->getLine(), "Expected \';\'.");
        token.print();
        return false;
    }
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(AstBlock *block) {
    AstReturnStmt *stmt = new AstReturnStmt;
    block->addStatement(stmt);
    
    if (!buildExpression(stmt, DataType::Void)) return false;
    
    return true;
}

