#pragma once

#include <string>
#include <fstream>
#include <stack>

#include <parser/base_lex.hpp>

//
// Represents token data
//
enum token {
    t_eof = 0,
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
struct Lex : BaseLex {
    explicit Lex(std::string input);
    void unget(int t) override;
    int get_next() override;
    void debug_token(int t) override;
    
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

