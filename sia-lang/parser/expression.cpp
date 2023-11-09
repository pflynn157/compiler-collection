#include <memory>
#include <string>

#include "parser.hpp"

//
// The main expression parser
//
std::shared_ptr<AstExpression> Parser::parse_expression(std::shared_ptr<AstBlock> block, token stop, bool is_list) {
    auto ctx = std::make_shared<ExprContext>();
    
    token t = lex->get_next();
    while (t != t_eof && t != stop) {
        switch (t) {
            // Integer literals
            case t_int_literal: {
                int value = std::stoi(lex->value);
                auto i = std::make_shared<AstI32>(value);
                ctx->output.push(i);
            } break;
        }
        
        t = lex->get_next();
    }
    
    // Apply associativity (associtivty property of math)
    applyAssoc(ctx);
    
    // We should not end with EOF
    if (t == t_eof) {
        syntax->addError(lex->line_number, "Invalid EOF in expression.");
        return nullptr;
    }
    
    // Build (if needed) and return the final structure
    if (ctx->output.empty()) return nullptr;
    if (is_list) {
        // TODO
    }
    
    std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
    return expr;
}

