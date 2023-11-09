#pragma once

#include <string>
#include <fstream>
#include <stack>

//
// Represents token data
//
enum token {
    t_none,
    t_eof,
    
    t_module,
    t_func,
    t_of,
    t_is,
    t_return,
    t_end,
    
    t_int,
    
    t_period,
    t_lparen,
    t_rparen,
    
    t_id,
    t_int_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    token get_next();
    void debug_token(token t);
    
    std::string value = "";
    int line_number = 0;
private:
    std::ifstream reader;
    std::string buffer = "";
    std::stack<token> token_stack;
    
    // Internal functions
    bool is_symbol(char c);
    token get_symbol(char c);
    bool is_integer();
};

