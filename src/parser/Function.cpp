//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

// Returns the function arguments
bool Parser::getFunctionArgs(std::shared_ptr<AstBlock> block, std::vector<Var> &args) {
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
            
            tk = lex_get_next(scanner);
            if (tk == t_comma) {
                tk = lex_get_next(scanner);
            }
            
            args.push_back(v);
            block->addSymbol(v.name, v.type);
        }
    } else {
        lex_rewind(scanner, tk);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(token startToken, std::string className) {
    localConsts.clear();
    
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
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    if (!getFunctionArgs(block, args)) return false;

    // Check to see if there's any return type
    //std::string retName = "";       // TODO: Do we need this?
    tk = lex_get_next(scanner);
    std::shared_ptr<AstDataType> dataType;
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
        std::shared_ptr<AstExternFunction> ex = std::make_shared<AstExternFunction>(funcName);
        ex->args = args;
        ex->data_type = dataType;
        tree->addGlobalStatement(ex);
        return true;
    }
    
    std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(funcName);
    func->data_type = dataType;
    func->args = args;
    tree->addGlobalStatement(func);
    func->block->mergeSymbols(block);
    
    // Build the body
    int stopLayer = 0;
    if (!buildBlock(func->block)) return false;
    
    // Make sure we end with a return statement
    V_AstType lastType = func->block->getBlock().back()->type;
    if (lastType == V_AstType::Return) {
        std::shared_ptr<AstStatement> ret = func->block->getBlock().back();
        if (func->data_type->type == V_AstType::Void && ret->hasExpression()) {
            syntax->addError(0, "Cannot return from void function.");
            return false;
        } else if (!ret->hasExpression()) {
            syntax->addError(0, "Expected return value.");
            return false;
        }
    } else {
        if (func->data_type->type == V_AstType::Void) {
            func->addStatement(std::make_shared<AstReturnStmt>());
        } else {
            syntax->addError(0, "Expected return statement.");
            return false;
        }
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, token idToken) {
    // Make sure the function exists
    if (!isFunc(lex_get_id(scanner))) {
        syntax->addError(0, "Unknown function.");
        return false;
    }

    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(lex_get_id(scanner));
    block->addStatement(fc);
    
    std::shared_ptr<AstExpression> args = buildExpression(block, nullptr, t_semicolon, false, true);
    if (!args) return false;
    fc->expression = args;
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstReturnStmt> stmt = std::make_shared<AstReturnStmt>();
    block->addStatement(stmt);
    
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr);
    if (!arg) return false;
    stmt->expression = arg;
    
    return true;
}

