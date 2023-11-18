#include <memory>
#include <string>

#include "parser.hpp"

bool Parser::is_constant(int tk) {
    switch (tk) {
        case t_int_literal:
        case t_string_literal: return true;
    
        default: {}
    }
    
    return false;
}

bool Parser::is_id(int tk) {
    if (tk == t_id) return true;
    return false;
}

bool Parser::is_list_delim(int tk) {
    if (tk == t_comma) return true;
    return false;
}

std::shared_ptr<AstExpression> Parser::build_constant(int tk) {
    switch (tk) {
        case t_int_literal: {
            int value = std::stoi(lex->value);
            auto i = std::make_shared<AstInt>(value);
            return i;
        }
        
        case t_string_literal: return std::make_shared<AstString>(lex->value);
        
        default: {}
    }
    
    return nullptr;
}

bool Parser::build_identifier(std::shared_ptr<AstBlock> block, int tk, std::shared_ptr<ExprContext> ctx) {
    ctx->lastWasOp = false;

    std::string name = lex->value;
    if (block->isVar(name)) {
        auto i = std::make_shared<AstID>(name);
        ctx->output.push(i);
    } else if (block->isFunc(name)) {
    
    } else {
        syntax->addError(lex->line_number, "Invalid identifier in expression.");
        return false;
    }
    return true;
}

