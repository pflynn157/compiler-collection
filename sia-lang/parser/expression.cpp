#include <memory>
#include <string>

#include "parser.hpp"

//
// The main expression parser
//
std::shared_ptr<AstExpression> Parser::parse_expression(std::shared_ptr<AstBlock> block, token stop, bool is_list) {
    auto ctx = std::make_shared<ExprContext>();
    std::vector<std::shared_ptr<AstExpression>> list;
    
    token t = lex->get_next();
    while (t != t_eof && t != stop) {
        switch (t) {
            // Integer literals
            case t_int_literal: {
                int value = std::stoi(lex->value);
                auto i = std::make_shared<AstI32>(value);
                ctx->output.push(i);
            } break;
            
            // String literals
            case t_string_literal: {
                auto s = std::make_shared<AstString>(lex->value);
                ctx->output.push(s);
            } break;
            
            // Identifiers
            case t_id: {
                std::string name = lex->value;
                if (block->isVar(name)) {
                    auto i = std::make_shared<AstID>(name);
                    ctx->output.push(i);
                } else if (block->isFunc(name)) {
                
                } else {
                    syntax->addError(lex->line_number, "Invalid identifier in expression.");
                    return nullptr;
                }
            } break;
            
            // Commas- for lists
            case t_comma: {
                applyAssoc(ctx);
                
                std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
                ctx->output.pop();
                
                list.push_back(expr);
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
    if (ctx->output.empty()) {
        if (is_list) return std::make_shared<AstExprList>();
        return nullptr;
    }
    if (is_list) {
        std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
        list.push_back(expr);
        
        auto ast_list = std::make_shared<AstExprList>();
        for (auto const &item : list) {
            ast_list->add_expression(item);
        }
        
        return ast_list;
    }
    
    std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
    return expr;
}

