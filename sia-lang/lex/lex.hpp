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
    t_scalar,
    t_iter,
    t_do,
    t_by,
    
    t_void,
    t_int,
    
    t_period,
    t_lparen,
    t_rparen,
    t_annot,
    t_colon,
    t_assign,
    t_comma,
    
    t_id,
    t_int_literal,
    t_string_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    void unget(token t);
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

