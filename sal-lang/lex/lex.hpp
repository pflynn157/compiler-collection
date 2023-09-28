#pragma once

#include <string>
#include <fstream>

//
// Token definitions
//
enum token {
    t_none,
    t_eof,
    
    // Keywords
    t_return,
    
    // Symbols
    t_nl,
    
    // Literals
    t_int_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    token get_next();
    void print(token t);
    
    // Accessible member variables
    std::string value = "";
    
private:
    std::ifstream reader;
    std::string buffer = "";
};

