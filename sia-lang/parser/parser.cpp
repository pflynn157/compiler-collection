#include "parser.hpp"

//
// Set everything up
//
Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
}

//
// The main parse loop
//
bool Parser::parse() {

    //
    // Print any warnings and errors
    //
    syntax->printWarnings();
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    return true;
}

