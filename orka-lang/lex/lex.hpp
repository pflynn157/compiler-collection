#pragma once

#include <fstream>
#include <string>
#include <stack>

// Represents a token
enum TokenType {
    t_eof,
    t_none,
    
    t_extern,
    t_func,
    t_struct,
    t_end,
    t_return,
    t_var,
    t_const,
    t_bool, t_char, t_string,
    t_i8, t_u8, t_i16, t_u16,
    t_i32, t_u32, t_i64, t_u64,
    t_if, t_elif, t_else, t_while,
    t_is, t_then, t_do,
    t_break, t_continue,
    t_import,
    t_true, t_false,
    t_lgand, t_lgor,
    t_sizeof, t_for,
    t_in, t_step,
    
    t_dot, t_semicolon, t_comma,
    t_lparen, t_rparen, t_lbracket, t_rbracket,
    t_plus, t_minus, t_mul, t_div, t_mod,
    t_and, t_or, t_xor,
    t_colon,
    t_gt, t_gte,
    t_lt, t_lte,
    t_eq, t_neq,
    t_assign,
    t_arrow,
    t_range,
    
    t_id,
    t_int_literal,
    t_string_literal,
    t_char_literal,
    t_float_literal,
};

struct Token {
    TokenType type;
    std::string id_val;
    char i8_val;
    int i32_val;
    double float_val = 0;
    
    Token();
    void print();
};

// The main lexical analysis class
class Scanner {
public:
    explicit Scanner(std::string input);
    ~Scanner();
    
    void rewind(Token token);
    Token getNext();
    
    std::string getRawBuffer();
    int getLine() { return currentLine; }
    
    bool isEof() { return reader.eof(); }
    bool isError() { return error; }
private:
    std::ifstream reader;
    bool error = false;
    std::stack<Token> token_stack;
    
    // Control variables for the scanner
    std::string rawBuffer = "";
    std::string buffer = "";
    bool inQuote = false;
    int currentLine = 1;
    bool skipNextLineCount = false;
    
    // Functions
    bool isSymbol(char c);
    TokenType getKeyword();
    TokenType getSymbol(char c);
    bool isInt();
    bool isHex();
    bool isFloat();
};


