#pragma once

#include <string>
#include <fstream>
#include <stack>

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
    t_id,
    t_int_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    token get_next();
    bool is_symbol(char c);
    token get_symbol(char c);
    bool is_int();
    void print(token t);
    
    // Accessible member variables
    std::string value = "";
    std::stack<token> stack;
    
private:
    std::ifstream reader;
    std::string buffer = "";
};

