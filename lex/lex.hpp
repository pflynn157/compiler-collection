#pragma once

#include <string>
#include <fstream>
#include <stack>

//
// Represents token data
//
enum token {
    t_eof,
    t_none,
    
    ///LEX_KEYWORDS
    
    ///LEX_SYMBOLS
    
    t_id,
    t_int_literal,
    t_string_literal,
    t_char_literal,
    t_float_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    void unget(int t);
    token get_next();
    void debug_token(token t);
    
    std::string value = "";
    int i_value = 0;
    int line_number = 0;
private:
    std::ifstream reader;
    std::string buffer = "";
    std::stack<token> token_stack;
    
    // Internal functions
    bool is_symbol(char c);
    token get_symbol(char c);
    bool is_integer();
    bool is_hex();
};

