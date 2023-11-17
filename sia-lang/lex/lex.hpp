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

