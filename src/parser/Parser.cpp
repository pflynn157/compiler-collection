//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include <parser/Parser.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

Parser::Parser(std::string input) {
    this->input = input;
    
    std::string input_str = "";
    std::ifstream reader(input.c_str());
    if (reader.is_open()) {
        std::string line = "";
        while (std::getline(reader, line)) {
            input_str += line + '\n';
        }
    }
    scanner = lex_init_string(strdup(input_str.c_str()));
    
    tree = new AstTree(input);
    syntax = new ErrorManager;
    
    // Add the built-in functions
    //string malloc(string)
    funcs.push_back("malloc");
    AstExternFunction *FT1 = new AstExternFunction("malloc");
    FT1->addArgument(Var(AstBuilder::buildInt32Type(), "size"));
    FT1->setDataType(AstBuilder::buildStringType());
    tree->addGlobalStatement(FT1);
    
    //println(string)
    funcs.push_back("println");
    AstExternFunction *FT2 = new AstExternFunction("println");
    FT2->setVarArgs();
    FT2->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT2->setDataType(AstBuilder::buildVoidType());
    tree->addGlobalStatement(FT2);
    
    //print(string)
    funcs.push_back("print");
    AstExternFunction *FT3 = new AstExternFunction("print");
    FT3->setVarArgs();
    FT3->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT3->setDataType(AstBuilder::buildVoidType());
    tree->addGlobalStatement(FT3);
    
    //i32 strlen(string)
    funcs.push_back("strlen");
    AstExternFunction *FT4 = new AstExternFunction("strlen");
    FT4->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT4->setDataType(AstBuilder::buildInt32Type());
    tree->addGlobalStatement(FT4);
    
    //i32 stringcmp(string, string)
    funcs.push_back("stringcmp");
    AstExternFunction *FT5 = new AstExternFunction("stringcmp");
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->setDataType(AstBuilder::buildInt32Type());
    tree->addGlobalStatement(FT5);
    
    //string strcat_str(string, string)
    funcs.push_back("strcat_str");
    AstExternFunction *FT6 = new AstExternFunction("strcat_str");
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->setDataType(AstBuilder::buildStringType());
    tree->addGlobalStatement(FT6);
    
    //string strcat_char(string, char)
    funcs.push_back("strcat_char");
    AstExternFunction *FT7 = new AstExternFunction("strcat_char");
    FT7->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT7->addArgument(Var(AstBuilder::buildCharType(), "c"));
    FT7->setDataType(AstBuilder::buildStringType());
    tree->addGlobalStatement(FT7);
}

Parser::~Parser() {
    lex_close(scanner);
    delete syntax;
}

bool Parser::parse() {
    token tk;
    do {
        tk = lex_get_next(scanner);
        bool code = true;
        
        switch (tk) {
            case t_extern:
            case t_func: {
                code = buildFunction(tk);
            } break;
            
            case t_const: code = buildConst(true); break;
            case t_struct: code = buildStruct(); break;
            
            case t_eof: break;
            
            default: {
                syntax->addError(0, "Invalid token in global scope.");
                lex_debug(tk, scanner);
                code = false;
            }
        }
        
        if (!code) break;
    } while (tk != t_eof);
    
    // Check for errors, and print if so
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    syntax->printWarnings();
    return true;
}

// Builds a statement block
bool Parser::buildBlock(AstBlock *block, AstNode *parent) {
    token tk = lex_get_next(scanner);
    while (tk != t_end && tk != t_eof) {
        bool code = true;
        bool end = false;
        
        switch (tk) {
            case t_var: code = buildVariableDec(block); break;
            case t_struct: code = buildStructDec(block); break;
            case t_const: code = buildConst(false); break;
            
            case t_id: {
                token idtoken = tk;
                tk = lex_get_next(scanner);
                
                if (tk == t_assign || tk == t_lbracket || tk == t_dot) {
                    lex_rewind(scanner, tk);
                    lex_rewind(scanner, idtoken);
                    code = buildVariableAssign(block, idtoken);
                } else if (tk == t_lparen) {
                    code = buildFunctionCallStmt(block, idtoken);
                } else {
                    syntax->addError(0, "Invalid use of identifier.");
                    //token.print();
                    return false;
                }
            } break;
            
            case t_return: code = buildReturn(block); break;
            
            // Handle conditionals
            case t_if: code = buildConditional(block); break;
            case t_elif: {
                AstIfStmt *condParent = static_cast<AstIfStmt *>(parent);
                code = buildConditional(condParent->getFalseBlock());
                end = true;
            } break;
            case t_else: {
                AstIfStmt *condParent = static_cast<AstIfStmt *>(parent);
                buildBlock(condParent->getFalseBlock());
                end = true;
            } break;
            
            // Handle loops
            case t_while: code = buildWhile(block); break;
            case t_break: code = buildLoopCtrl(block, true); break;
            case t_continue: code = buildLoopCtrl(block, false); break;
            
            default: {
                syntax->addError(0, "Invalid token in block.");
                //token.print();
                return false;
            }
        }
        
        if (end) break;
        if (!code) return false;
        tk = lex_get_next(scanner);
    }
    
    return true;
}

// This is meant mainly for literals; it checks to make sure all the types in
// the expression agree in type. LLVM will have a problem if not
std::shared_ptr<AstExpression> Parser::checkExpression(std::shared_ptr<AstExpression> expr, AstDataType *varType) {
    if (!varType) return expr;

    switch (expr->getType()) {
        case V_AstType::I32L: {
            // Change to byte literals
            if (varType->getType() == V_AstType::Int8) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI8> byte = std::make_shared<AstI8>(i32->getValue());
                expr = byte;
                
            // Change to word literals
            } else if (varType->getType() == V_AstType::Int16) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI16> i16 = std::make_shared<AstI16>(i32->getValue());
                expr = i16;
                
            // Change to qword literals
            } else if (varType->getType() == V_AstType::Int64) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI64> i64 = std::make_shared<AstI64>(i32->getValue());
                expr = i64;
            }
        } break;
            
        default: {}
    }
    
    return expr;
}

// The debug function for the scanner
void Parser::debugScanner() {
    std::cout << "Debugging scanner..." << std::endl;
    
    token t;
    do {
        t = lex_get_next(scanner);
        lex_debug(t, scanner);
    } while (t != t_eof);
}

// Checks to see if a string is a constant
int Parser::isConstant(std::string name) {
    if (globalConsts.find(name) != globalConsts.end()) {
        return 1;
    }
    
    if (localConsts.find(name) != localConsts.end()) {
        return 2;
    }
    
    return 0;
}

bool Parser::isFunc(std::string name) {
    if (std::find(funcs.begin(), funcs.end(), name) != funcs.end()) {
        return true;
    }
    return false;
}

//
// Builds a data type from the token stream
//
AstDataType *Parser::buildDataType(bool checkBrackets) {
    token tk = lex_get_next(scanner);
    AstDataType *dataType = nullptr;
    
    switch (tk) {
        case t_bool: dataType = AstBuilder::buildBoolType(); break;
        case t_char: dataType = AstBuilder::buildCharType(); break;
        case t_i8: dataType = AstBuilder::buildInt8Type(); break;
        case t_u8: dataType = AstBuilder::buildInt8Type(true); break;
        case t_i16: dataType = AstBuilder::buildInt16Type(); break;
        case t_u16: dataType = AstBuilder::buildInt16Type(true); break;
        case t_i32: dataType = AstBuilder::buildInt32Type(); break;
        case t_u32: dataType = AstBuilder::buildInt32Type(true); break;
        case t_i64: dataType = AstBuilder::buildInt64Type(); break;
        case t_u64: dataType = AstBuilder::buildInt64Type(true); break;
        case t_string: dataType = AstBuilder::buildStringType(); break;
        
        case t_id: {
            bool isStruct = false;
            for (auto s : tree->getStructs()) {
                if (s->getName() == lex_get_id(scanner)) {
                    isStruct = true;
                    break;
                }
            }
                
            if (isStruct) {
                dataType = AstBuilder::buildStructType(lex_get_id(scanner));
            }
        } break;
        
        default: {}
    }

    if (checkBrackets) {
        tk = lex_get_next(scanner);
        if (tk == t_lbracket) {
            tk = lex_get_next(scanner);
            if (tk != t_rbracket) {
                syntax->addError(0, "Invalid pointer type.");
                return nullptr;
            }
            
            dataType = AstBuilder::buildPointerType(dataType);
        } else {
            lex_rewind(scanner, tk);
        }
    }
    
    return dataType;
}

