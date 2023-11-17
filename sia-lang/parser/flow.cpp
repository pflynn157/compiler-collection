#include <ast/ast_builder.hpp>

#include "parser.hpp"

//
// Parses an iter (for) loop
// Syntax: iter <index:id> of <expr>:<expr> do BLOCK
//
void Parser::parse_iter(std::shared_ptr<AstBlock> block) {
    // Index variable
    consume_token(t_id, "Expected index variable in iter loop.");
    std::string idx_name = lex->value;
    
    // Of
    consume_token(t_of, "Expected \"of\" in iter loop.");
    
    // Bound expressions
    auto dtype = AstBuilder::buildInt32Type();
    std::shared_ptr<AstExpression> start = buildExpression(block, dtype, t_colon);
    std::shared_ptr<AstExpression> end = buildExpression(block, dtype, t_by);
    std::shared_ptr<AstExpression> step = buildExpression(block, dtype, t_do);
    
    // Build the AST node
    auto iter = std::make_shared<AstForStmt>();
    iter->index = std::make_shared<AstID>(idx_name);
    iter->start = start;
    iter->end = end;
    iter->step = step;
    iter->data_type = dtype;
    block->addStatement(iter);
    
    // Build the loop block
    iter->block->addSymbol(idx_name, iter->data_type);
    iter->block->mergeSymbols(block);
    parse_block(iter->block);
}

