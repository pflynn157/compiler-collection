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
    tree->block->funcs.push_back(func_name);
    
    // Build the function body
    func->block->mergeSymbols(tree->block);
    parse_block(func->block);
    
    // If we have a function function, add a return statement
    if (data_type->type == V_AstType::Void) {
        auto ret = std::make_shared<AstReturnStmt>();
        func->block->addStatement(ret);
    }
}

//
// Parses a return statement
//
void Parser::parse_return(std::shared_ptr<AstBlock> block) {
    auto ret = std::make_shared<AstReturnStmt>();
    ret->expression = parse_expression(block);
    block->addStatement(ret);
}

//
// Parses a function call
//
void Parser::parse_function_call(std::shared_ptr<AstBlock> block, std::string name) {
    if (!block->isFunc(name)) {
        syntax->addError(lex->line_number, "Invalid function in call: " + name);
        return;
    }

    auto fc = std::make_shared<AstFuncCallStmt>(name);
    fc->expression = parse_expression(block, t_period, true);
    block->addStatement(fc);
}

