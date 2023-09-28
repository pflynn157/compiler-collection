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
            
            case t_func: code = parse_function(block, t); break;
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
// Parses a function definition
//
bool Parser::parse_function(std::shared_ptr<AstBlock> block, token start) {
    token t = lex->get_next();
    std::string name = lex->value;
    if (t != t_id) {
        syntax->addError(0, "Expected function name.");
        lex->print(t);
        return false;
    }
    
    // Arguments
    t = lex->get_next();
    if (t != t_lcbrace) {
        syntax->addError(0, "Expected opening \'{\'.");
        lex->print(t);
        return false;
    }
    t = lex->get_next();
    while (t != t_rcbrace) {
        t = lex->get_next();
    }
    
    // If we have a function, get the return
    std::shared_ptr<AstDataType> dtype;
    if (start == t_func) {
        t = lex->get_next();
        if (t != t_of) {
            syntax->addError(0, "Expected \"of\" in function declaration. Use \"def\" for void functions.");
            lex->print(t);
            return false;
        }
    
        // TODO: Do this properly
        lex->get_next(); lex->get_next();
        
        dtype = std::make_shared<AstDataType>(V_AstType::Int32);
    } else {
        dtype = std::make_shared<AstDataType>(V_AstType::Void);
    }

    // Build the AST node
    std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(name);
    func->data_type = dtype;
    block->addStatement(func);
    
    parse_block(func->block);
    
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
std::shared_ptr<AstExpression> Parser::parse_expression(std::shared_ptr<AstBlock> block, token stop) {
    std::shared_ptr<ExprContext> context = std::make_shared<ExprContext>();
    
    token t = lex->get_next();
    while (t != t_eof && t != stop) {
        switch (t) {
            case t_int_literal: {
                int val = std::stoi(lex->value);
                std::shared_ptr<AstI32> i = std::make_shared<AstI32>(val);
                context->output.push(i);
            } break;
            
            default: {
                syntax->addError(0, "Unknown token in expression");
                lex->print(t);
                return nullptr;
            }
        }
        
        t = lex->get_next();
    }
    
    if (context->output.empty()) return nullptr;
    return context->output.top();
}

