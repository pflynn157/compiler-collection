//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

// Returns the function arguments
bool Parser::getFunctionArgs(std::vector<Var> &args) {
    token tk = lex_get_next(scanner);
    if (tk == t_lparen) {
        tk = lex_get_next(scanner);
        while (tk != t_eof && tk != t_rparen) {
            token t1 = tk;
            char *name = lex_get_id(scanner);
            token t2 = lex_get_next(scanner);
            Var v;
            
            if (t1 != t_id) {
                syntax->addError(0, "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2 != t_colon) {
                syntax->addError(0, "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            v.type = buildDataType();
            v.name = name;
            vars.push_back(name);
            
            tk = lex_get_next(scanner);
            if (tk == t_comma) {
                tk = lex_get_next(scanner);
            }
            
            args.push_back(v);
            typeMap[v.name] = v.type;
        }
    } else {
        lex_rewind(scanner, tk);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(token startToken, std::string className) {
    typeMap.clear();
    localConsts.clear();
    vars.clear();
    
    token tk;
    bool isExtern = false;

    // Handle extern function
    if (startToken == t_extern) {
        isExtern = true;
    }

    // Make sure we have a function name
    tk = lex_get_next(scanner);
    std::string funcName = lex_get_id(scanner);
    
    if (tk != t_id) {
        syntax->addError(0, "Expected function name.");
        return false;
    }
    
    // Get arguments
    std::vector<Var> args;
    if (!getFunctionArgs(args)) return false;

    // Check to see if there's any return type
    //std::string retName = "";       // TODO: Do we need this?
    tk = lex_get_next(scanner);
    AstDataType *dataType;
    if (tk == t_arrow) {
        dataType = buildDataType();
        tk = lex_get_next(scanner);
    }
    else dataType = AstBuilder::buildVoidType();
    
    // Do syntax error check
    if (tk == t_semicolon && !isExtern) {
        syntax->addError(0, "Expected \';\' for extern function.");
        return false;
    } else if (tk == t_is && isExtern) {
        syntax->addError(0, "Expected \'is\' keyword.");
        return false;
    }

    // Create the function object
    funcs.push_back(funcName);
    
    if (isExtern) {
        AstExternFunction *ex = new AstExternFunction(funcName);
        ex->setArguments(args);
        ex->setDataType(dataType);
        tree->addGlobalStatement(ex);
        return true;
    }
    
    AstFunction *func = new AstFunction(funcName);
    func->setDataType(dataType);
    func->setArguments(args);
    tree->addGlobalStatement(func);
    
    // Build the body
    int stopLayer = 0;
    if (!buildBlock(func->getBlock())) return false;
    
    // Make sure we end with a return statement
    V_AstType lastType = func->getBlock()->getBlock().back()->getType();
    if (lastType == V_AstType::Return) {
        AstStatement *ret = func->getBlock()->getBlock().back();
        if (func->getDataType()->getType() == V_AstType::Void && ret->hasExpression()) {
            syntax->addError(0, "Cannot return from void function.");
            return false;
        } else if (!ret->hasExpression()) {
            syntax->addError(0, "Expected return value.");
            return false;
        }
    } else {
        if (func->getDataType()->getType() == V_AstType::Void) {
            func->addStatement(new AstReturnStmt);
        } else {
            syntax->addError(0, "Expected return statement.");
            return false;
        }
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(AstBlock *block, token idToken) {
    // Make sure the function exists
    if (!isFunc(lex_get_id(scanner))) {
        syntax->addError(0, "Unknown function.");
        return false;
    }

    AstFuncCallStmt *fc = new AstFuncCallStmt(lex_get_id(scanner));
    block->addStatement(fc);
    
    AstExpression *args = buildExpression(nullptr, t_semicolon, false, true);
    if (!args) return false;
    fc->setExpression(args);
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(AstBlock *block) {
    AstReturnStmt *stmt = new AstReturnStmt;
    block->addStatement(stmt);
    
    AstExpression *arg = buildExpression(nullptr);
    if (!arg) return false;
    stmt->setExpression(arg);
    
    return true;
}

