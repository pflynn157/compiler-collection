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
#include <lex/lex.hpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::shared_ptr<AstBlock> block, std::vector<Var> &args) {
    Token tk = scanner->getNext();
    if (tk.type == t_lparen) {
        tk = scanner->getNext();
        while (tk.type != t_eof && tk.type != t_rparen) {
            Token t1 = tk;
            std::string name = t1.id_val;
            Token t2 = scanner->getNext();
            Var v;
            
            if (t1.type != t_id) {
                syntax->addError(0, "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2.type != t_colon) {
                syntax->addError(0, "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            v.type = buildDataType();
            v.name = name;
            
            tk = scanner->getNext();
            if (tk.type == t_comma) {
                tk = scanner->getNext();
            }
            
            args.push_back(v);
            block->addSymbol(v.name, v.type);
        }
    } else {
        scanner->rewind(tk);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(Token startToken, std::string className) {
    Token tk;
    bool isExtern = false;

    // Handle extern function
    if (startToken.type == t_extern) {
        isExtern = true;
    }

    // Make sure we have a function name
    tk = scanner->getNext();
    std::string funcName = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected function name.");
        return false;
    }
    
    // Get arguments
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    if (!getFunctionArgs(block, args)) return false;

    // Check to see if there's any return type
    //std::string retName = "";       // TODO: Do we need this?
    tk = scanner->getNext();
    std::shared_ptr<AstDataType> dataType;
    if (tk.type == t_arrow) {
        dataType = buildDataType();
        tk = scanner->getNext();
    }
    else dataType = AstBuilder::buildVoidType();
    
    // Do syntax error check
    if (tk.type == t_semicolon && !isExtern) {
        syntax->addError(0, "Expected \';\' for extern function.");
        return false;
    } else if (tk.type == t_is && isExtern) {
        syntax->addError(0, "Expected \'is\' keyword.");
        return false;
    }

    // Create the function object
    tree->block->funcs.push_back(funcName);
    
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
    func->block->mergeSymbols(tree->block);
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
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken) {
    // Make sure the function exists
    if (!block->isFunc(idToken.id_val)) {
        syntax->addError(0, "Unknown function.");
        return false;
    }

    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(idToken.id_val);
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

