#include <iostream>

#include "lex.hpp"

//
// Initialize the lexer
//
Lex::Lex(std::string input) {
    reader = std::ifstream(input);
}

//
// Get the next token in the stream
//
token Lex::get_next() {
    if (reader.eof()) {
        return t_eof;
    }
    
    while (!reader.eof()) {
        char c = reader.get();
    }
    
    return t_eof;
}

//
// Our lexical debug function
//
void Lex::print(token t) {
    switch (t) {
        case t_eof: std::cout << "EOF" << std::endl; break;
    
        // Keywords
        case t_return: std::cout << "RETURN" << std::endl; break;

        // Symbols
        case t_nl: std::cout << "NL" << std::endl; break;

        // Literals
        case t_int_literal: std::cout << "VAL(" << value << ")" << std::endl; break;
        
        default: std::cout << "???" << std::endl;
    }
}

