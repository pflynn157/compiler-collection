#include "parser.hpp"

//
// Parses a scalar variable
//
void Parser::parse_scalar(std::shared_ptr<AstBlock> block) {
    // Variable name
    consume_token(t_id, "Expected variable name.");
    std::string name = lex->value;
    
    // Colon
    consume_token(t_colon, "Expected \':\' in variable declaration.");
    
    // Data type
    std::shared_ptr<AstDataType> data_type = get_data_type();
    
    // Build the variable declaration
    block->addSymbol(name, data_type);
    
    auto vd = std::make_shared<AstVarDec>(name, data_type);
    block->addStatement(vd);
    
    // Assignment and expression
    // -- Expression statement
    consume_token(t_assign, "Expected initial value in scalar assignment.");
    
    auto rval = parse_expression(block);
    auto lval = std::make_shared<AstID>(name);
    auto assign = std::make_shared<AstAssignOp>(lval, rval);
    auto stmt = std::make_shared<AstExprStatement>();
    stmt->expression = assign;
    block->addStatement(stmt);
}

