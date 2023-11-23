//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::shared_ptr<AstBlock> block, std::vector<Var> &args) {
    int tk = lex->get_next();
    if (tk == t_lparen) {
        tk = lex->get_next();
        while (tk != t_eof && tk != t_rparen) {
            int t1 = tk;
            std::string name = lex->value;
            int t2 = lex->get_next();
            Var v;
            
            if (t1 != t_id) {
                syntax->addError(lex->line_number, "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2 != t_colon) {
                syntax->addError(lex->line_number, "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            v.type = buildDataType();
            v.name = name;
            
            tk = lex->get_next();
            if (tk == t_comma) {
                tk = lex->get_next();
            }
            
            args.push_back(v);
            block->addSymbol(v.name, v.type);
        }
    } else {
        lex->unget(tk);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(int startToken, std::string className) {
    int tk;
    bool isExtern = false;

    // Handle extern function
    if (startToken == t_extern) {
        isExtern = true;
    }

    // Make sure we have a function name
    consume_token(t_id, "Expected function name.");
    std::string funcName = lex->value;
    
    // Get arguments
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    if (!getFunctionArgs(block, args)) return false;

    // Check to see if there's any return type
    //std::string retName = "";       // TODO: Do we need this?
    tk = lex->get_next();
    std::shared_ptr<AstDataType> dataType;
    if (tk == t_arrow) {
        dataType = buildDataType();
        tk = lex->get_next();
    }
    else dataType = AstBuilder::buildVoidType();
    
    // Do syntax error check
    if (tk == t_semicolon && !isExtern) {
        syntax->addError(lex->line_number, "Expected \';\' for extern function.");
        return false;
    } else if (tk == t_is && isExtern) {
        syntax->addError(lex->line_number, "Expected \'is\' keyword.");
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
    
    // For the Java backend
    func->routine = true;
    func->attr = Attr::Public;
    
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
// TODO: Pass variable name in from block parser
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, std::string fc_name) {
    // Make sure the function exists
    if (!ignore_invalid_funcs && !block->isFunc(fc_name)) {
        syntax->addError(0, "Unknown function.");
        return false;
    }

    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(fc_name);
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
    
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, t_semicolon);
    if (!arg) return false;
    stmt->expression = arg;
    
    return true;
}

