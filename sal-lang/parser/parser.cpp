#include "parser.hpp"

Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
}

//
// The main parse loop
//
bool Parser::parse() {
    parse_block(tree->block);
    
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
bool Parser::parse_block(std::shared_ptr<AstBlock> block) {
    token t = lex->get_next();
    bool code = true;
    
    while (t != t_eof) {
        switch (t) {
            case t_nl: break;
            
            case t_return: code = parse_return(block); break;
            
            default: {
                syntax->addError(0, "Unknown token in block");
                lex->print(t);
                return false;
            }
        }
        
        if (!code) return false;
        t = lex->get_next();
    }

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
        }
        
        t = lex->get_next();
    }
    
    if (context->output.empty()) return nullptr;
    return context->output.top();
}

