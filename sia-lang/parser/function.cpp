#include "parser.hpp"

//
// Parses a function
//
void Parser::parse_function() {
    // Function name
    consume_token(t_id, "Expected function name.");
    std::string func_name = generate_name(lex->value);
    
    // Function arguments
    consume_token(t_lparen, "Expected \'(\' after function name.");
    // Temporary- consume arguments
    token t = lex->get_next();
    while (t != t_rparen) t = lex->get_next();
    
    // Data type
    consume_token(t_of, "Expected \"of\" in function declaration.");
    std::shared_ptr<AstDataType> data_type = get_data_type();
    
    // "is"
    consume_token(t_is, "Expected \"is\" in function declaration.");
    
    // Build the AST node
    auto func = std::make_shared<AstFunction>(func_name, data_type);
    tree->block->addStatement(func);
    
    // Build the function body
}

