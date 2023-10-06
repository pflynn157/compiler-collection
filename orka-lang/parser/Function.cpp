//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::vector<Var> &args, std::shared_ptr<AstBlock> block) {
    Token token = scanner->getNext();
    if (token.type == LParen) {
        token = scanner->getNext();
        while (token.type != Eof && token.type != RParen) {
            Token t1 = token;
            Token t2 = scanner->getNext();
            Token t3 = scanner->getNext();
            Var v;
            
            if (t1.type != Id) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2.type != Colon) {
                syntax->addError(scanner->getLine(), "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            switch (t3.type) {
                case Bool: v.type = AstBuilder::buildVoidType(); break;
                case Char: v.type = AstBuilder::buildCharType(); break;
                case Byte: v.type = AstBuilder::buildInt8Type(); break;
                case UByte: v.type = AstBuilder::buildInt8Type(true); break;
                case Short: v.type = AstBuilder::buildInt16Type(); break;
                case UShort: v.type = AstBuilder::buildInt16Type(true); break;
                case Int: v.type = AstBuilder::buildInt32Type(); break;
                case UInt: v.type = AstBuilder::buildInt32Type(true); break;
                case Int64: v.type = AstBuilder::buildInt64Type(); break;
                case UInt64: v.type = AstBuilder::buildInt64Type(true); break;
                case Str: v.type = AstBuilder::buildStringType(); break;
                case Float: v.type = AstBuilder::buildFloat32Type(); break;
                case Double: v.type = AstBuilder::buildFloat64Type(); break;
                
                case Id: {
                    bool isStruct = false;
                    for (auto s : tree->structs) {
                        if (s->name == t3.id_val) {
                            isStruct = true;
                            break;
                        }
                    }
                    
                    if (isStruct) {
                        v.type = AstBuilder::buildStructType(t3.id_val);
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
                
                v.type = AstBuilder::buildPointerType(v.type);
            }
            
            args.push_back(v);
            block->symbolTable[v.name] = v.type;
        }
    } else {
        scanner->rewind(token);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(Token startToken, std::string className) {
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
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    std::vector<Var> args;
    if (className != "") {
        Var classV;
        classV.name = "this";
        classV.type = AstBuilder::buildStructType(className);
        args.push_back(classV);
        
        //typeMap["this"] = std::pair<DataType, DataType>(classV.type, classV.subType);
        block->symbolTable["this"] = classV.type;
    }
    
    if (!getFunctionArgs(args, block)) return false;

    // Check to see if there's any return type
    token = scanner->getNext();
    std::shared_ptr<AstDataType> funcType = AstBuilder::buildVoidType();
    std::string retName = "";
    
    if (token.type == Arrow) {
        token = scanner->getNext();
        switch (token.type) {
            case Int: funcType = AstBuilder::buildInt32Type(); break;
            case Str: funcType = AstBuilder::buildStringType(); break;
            
            case Id: {
                if (enums.find(token.id_val) != enums.end()) {
                    AstEnum dec = enums[token.id_val];
                    funcType = dec.type;
                    break;
                }
                
                bool isStruct = false;
                    for (auto s : tree->structs) {
                        if (s->name == token.id_val) {
                            isStruct = true;
                            break;
                        }
                    }
                    
                    if (isStruct) {
                        funcType = AstBuilder::buildStructType(token.id_val);
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
            
            funcType = AstBuilder::buildPointerType(funcType);
            
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
        std::shared_ptr<AstExternFunction> ex = std::make_shared<AstExternFunction>(funcName);
        ex->args = args;
        ex->data_type = funcType;
        tree->addGlobalStatement(ex);
        return true;
    }
    
    std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(funcName);
    func->data_type = funcType;
    func->args = args;
    
    //if (className == "") tree->addGlobalStatement(func);
    //else currentClass->addFunction(func);
    tree->addGlobalStatement(func);
    if (className != "") {
        std::string fullName = className + "_" + funcName;
        func->name = fullName;
    }
    
    // Build the body
    //int stopLayer = 0;
    //if (className != "") {
    //    stopLayer = 1;
    //    ++layer;
    //}
    
    if (!buildBlock(block)) return false;
    func->block = block;
    
    // Make sure we end with a return statement
    V_AstType lastType = func->block->block.back()->type;
    if (lastType == V_AstType::Return) {
        std::shared_ptr<AstStatement> ret = func->block->block.back();
        if (func->data_type->type == V_AstType::Void && ret->hasExpression()) {
            syntax->addError(scanner->getLine(), "Cannot return from void function.");
            return false;
        } else if (!ret->hasExpression()) {
            syntax->addError(scanner->getLine(), "Expected return value.");
            return false;
        }
    } else {
        if (func->data_type->type == V_AstType::Void) {
            func->addStatement(std::make_shared<AstReturnStmt>());
        } else {
            syntax->addError(scanner->getLine(), "Expected return statement.");
            return false;
        }
    }
    
    if (className != "") {
        std::shared_ptr<AstFunction> func2 = std::make_shared<AstFunction>(funcName);
        func2->data_type = funcType;
        func2->args = args;
        currentClass->addFunction(func2);
        
        std::shared_ptr<AstBlock> block2 = func->block;
        func2->block->addStatements(block2->block);
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken) {
    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(idToken.id_val);
    block->addStatement(fc);
    
    fc->expression = buildExpression(block, nullptr, RParen, Comma);
    if (!fc->expression) return false;
    
    Token token = scanner->getNext();
    if (token.type != SemiColon) {
        syntax->addError(scanner->getLine(), "Expected \';\'.");
        token.print();
        return false;
    }
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstReturnStmt> stmt = std::make_shared<AstReturnStmt>();
    block->addStatement(stmt);
    
    stmt->expression = buildExpression(block, nullptr);
    
    return true;
}

