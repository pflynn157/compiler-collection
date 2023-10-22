//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <algorithm>

#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

#include "parser.hpp"

Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
}

//
// The main parse loop
//
bool Parser::parse() {
    parse_block(tree->block, true);
    
    // Exit the parse loop
    if (syntax->errorsPresent()) {
        syntax->printWarnings();
        syntax->printErrors();
        return false;
    }
    
    return true;
}

//
// Parses a block of code
//
bool Parser::parse_block(std::shared_ptr<AstBlock> block, bool is_global) {
    token t = lex->get_next();
    bool code = true;
    bool end = false;
    
    while (t != t_eof) {
        switch (t) {
            case t_dot: break;
            
            case t_extern:
            case t_def:
            case t_func: code = parse_function(block, t); break;
            
            case t_id: {
                std::string name = lex->value;
                parse_call(block, name);
            } break;
            
            case t_return: code = parse_return(block); break;
            
            case t_end: {
                if (is_global == false) end = true;
            } break;
            
            default: {
                syntax->addError(0, "Unknown token in block");
                lex->print(t);
                return false;
            }
        }
        
        if (!code) return false;
        if (end) break;
        t = lex->get_next();
    }

    return true;
}

//
// Consume a token and check if there is an error
//
bool Parser::consume_token(token expected, std::string error_msg) {
    token t = lex->get_next();
    if (t != expected) {
        syntax->addError(0, error_msg);
        lex->print(t);
        return false;
    }
    return true;
}

//
// Parses a data type
//
std::shared_ptr<AstDataType> Parser::parse_data_type() {
    token t = lex->get_next();
    switch (t) {
        case t_string: return AstBuilder::buildStringType();
        
        case t_numsym: {
            if (!consume_token(t_int_literal, "Expected integer after \'#\'."))
                return nullptr;
            int len = std::stoi(lex->value);
            switch (len) {
                case 8: return AstBuilder::buildInt8Type();
                case 16: return AstBuilder::buildInt16Type();
                case 32: return AstBuilder::buildInt32Type();
                case 64: return AstBuilder::buildInt64Type();
                
                default: {
                    syntax->addError(0, "Invalid integer length: " + std::to_string(len));
                    return nullptr;
                }
            }
        }
        
        default: {}
    }
    
    return nullptr;
}

//
// Parses a function definition
//
bool Parser::parse_function(std::shared_ptr<AstBlock> block, token start) {
    bool is_extern = false;
    if (start == t_extern) {
        is_extern = true;
        start = lex->get_next();
        if (start != t_def && start != t_func) {
            syntax->addError(0, "Expected \"def\" or \"func\" after extern.");
            lex->print(start);
            return false;
        }
    }

    // Function name
    token t = lex->get_next();
    std::string name = lex->value;
    if (t != t_id) {
        syntax->addError(0, "Expected function name.");
        lex->print(t);
        return false;
    }
    
    // Arguments
    std::vector<Var> args;
    bool varargs = false;
    
    t = lex->get_next();
    if (t != t_lcbrace) {
        syntax->addError(0, "Expected opening \'{\'.");
        lex->print(t);
        return false;
    }
    
    t = lex->get_next();
    while (t != t_rcbrace) {
        // Check variadiac arguments
        if (t == t_any) {
            varargs = true;
            t = lex->get_next();
            continue;
        }
        
        // Check comma
        if (t == t_comma) {
            t = lex->get_next();
            continue;
        }
    
        // ID
        std::string id_name = lex->value;
        if (t != t_id) {
            syntax->addError(0, "Expected identifier in function argument.");
            lex->print(t);
            return false;
        }
        
        // Colon
        if (!consume_token(t_colon, "Expected \':\' after identifier.")) return false;
        
        // Data type
        std::shared_ptr<AstDataType> dtype = parse_data_type();
        
        // Add to the arguments and get the next token
        Var var(dtype, id_name);
        args.push_back(var);
        
        t = lex->get_next();
    }
    
    // If we have a function, get the return
    std::shared_ptr<AstDataType> dtype;
    bool is_def = false;
    if (start == t_func) {
        if (!consume_token(t_of, "Expected \"of\" in function declaration. Use \"def\" for void functions."))
            return false;
        dtype = parse_data_type();
    } else {
        dtype = std::make_shared<AstDataType>(V_AstType::Void);
        is_def = true;
    }

    // Build the AST node
    if (is_extern) {
        std::shared_ptr<AstExternFunction> func = std::make_shared<AstExternFunction>(name);
        func->data_type = dtype;
        func->args = args;
        func->varargs = varargs;
        block->addStatement(func);
    } else {
        std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(name);
        func->data_type = dtype;
        func->args = args;
        block->addStatement(func);
        
        parse_block(func->block);
        
        if (is_def) {
            std::shared_ptr<AstReturnStmt> ret = std::make_shared<AstReturnStmt>();
            func->block->addStatement(ret);
        }
    }
    
    return true;
}

//
// Parses a function call statement
//
bool Parser::parse_call(std::shared_ptr<AstBlock> block, std::string name) {
    std::shared_ptr<AstFuncCallStmt> call = std::make_shared<AstFuncCallStmt>(name);
    call->expression = parse_expression(block, t_dot, true);
    block->addStatement(call);

    return true;
}

//
// Parses a return statement
//
bool Parser::parse_return(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstReturnStmt> ret = std::make_shared<AstReturnStmt>();
    ret->expression = parse_expression(block);
    block->addStatement(ret);

    return true;
}

//
// Parses an expression
//
std::shared_ptr<AstExpression> Parser::parse_expression(std::shared_ptr<AstBlock> block, token stop, bool is_list) {
    std::shared_ptr<ExprContext> context = std::make_shared<ExprContext>();
    
    token t = lex->get_next();
    while (t != t_eof && t != stop) {
        switch (t) {
            case t_int_literal: {
                int val = std::stoi(lex->value);
                std::shared_ptr<AstI32> i = std::make_shared<AstI32>(val);
                context->output.push(i);
            } break;
            
            case t_string_literal: {
                std::shared_ptr<AstString> s = std::make_shared<AstString>(lex->value);
                context->output.push(s);
            } break;
            
            case t_comma: break;
            
            default: {
                syntax->addError(0, "Unknown token in expression");
                lex->print(t);
                return nullptr;
            }
        }
        
        t = lex->get_next();
    }
    
    if (is_list) {
        std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
        while (!context->output.empty()) {
            auto val = context->output.top();
            context->output.pop();
            list->list.push_back(val);
        }
        std::reverse(list->list.begin(), list->list.end());
        return list;
    }
    
    if (context->output.empty()) return nullptr;
    return context->output.top();
}

