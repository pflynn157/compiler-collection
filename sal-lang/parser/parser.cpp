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
    while (t != t_eof) {
        t = lex->get_next();
    }

    return true;
}

