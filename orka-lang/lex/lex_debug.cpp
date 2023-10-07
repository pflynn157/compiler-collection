#include <iostream>

#include "lex.hpp"

void Token::print() {
    switch (this->type) {
        case t_none: std::cout << "???" << std::endl; break;
        case t_eof: std::cout << "EOF" << std::endl; break;
        
        case t_extern: std::cout << "extern" << std::endl; break;
        case t_func: std::cout << "func" << std::endl; break;
        case t_struct: std::cout << "struct" << std::endl; break;
        case t_end: std::cout << "end" << std::endl; break;
        case t_return: std::cout << "return" << std::endl; break;
        case t_var: std::cout << "var" << std::endl; break;
        case t_const: std::cout << "const" << std::endl; break;
        case t_bool: std::cout << "bool" << std::endl; break;
        case t_char: std::cout << "char" << std::endl; break;
        case t_string: std::cout << "string" << std::endl; break;
        case t_i8: std::cout << "i8" << std::endl; break;
        case t_u8: std::cout << "u8" << std::endl; break;
        case t_i16: std::cout << "i16" << std::endl; break;
        case t_u16: std::cout << "u16" << std::endl; break;
        case t_i32: std::cout << "i32" << std::endl; break;
        case t_u32: std::cout << "u32" << std::endl; break;
        case t_i64: std::cout << "i64" << std::endl; break;
        case t_u64: std::cout << "u64" << std::endl; break;
        case t_if: std::cout << "if" << std::endl; break;
        case t_elif: std::cout << "elif" << std::endl; break;
        case t_else: std::cout << "else" << std::endl; break;
        case t_while: std::cout << "while" << std::endl; break;
        case t_is: std::cout << "is" << std::endl; break;
        case t_then: std::cout << "then" << std::endl; break;
        case t_do: std::cout << "do" << std::endl; break;
        case t_break: std::cout << "break" << std::endl; break;
        case t_continue: std::cout << "continue" << std::endl; break;
        case t_import: std::cout << "import" << std::endl; break;
        case t_true: std::cout << "true" << std::endl; break;
        case t_false: std::cout << "false" << std::endl; break;
        case t_lgand: std::cout << "and" << std::endl; break;
        case t_lgor: std::cout << "or" << std::endl; break;
        case t_sizeof: std::cout << "sizeof" << std::endl; break;
        case t_for: std::cout << "for" << std::endl; break;
        case t_in: std::cout << "in" << std::endl; break;
        case t_step: std::cout << "step" << std::endl; break;
        
        case t_dot: std::cout << "." << std::endl; break;
        case t_semicolon: std::cout << ";" << std::endl; break;
        case t_comma: std::cout << "," << std::endl; break;
        case t_lparen: std::cout << "(" << std::endl; break;
        case t_rparen: std::cout << ")" << std::endl; break;
        case t_lbracket: std::cout << "[" << std::endl; break;
        case t_rbracket: std::cout << "]" << std::endl; break;
        case t_plus: std::cout << "+" << std::endl; break;
        case t_minus: std::cout << "-" << std::endl; break;
        case t_arrow: std::cout << "->" << std::endl; break;
        case t_mul: std::cout << "*" << std::endl; break;
        case t_div: std::cout << "/" << std::endl; break;
        case t_mod: std::cout << "%" << std::endl; break;
        case t_and: std::cout << "&" << std::endl; break;
        case t_or: std::cout << "|" << std::endl; break;
        case t_xor: std::cout << "^" << std::endl; break;
        case t_colon: std::cout << ":" << std::endl; break;
        case t_assign: std::cout << ":=" << std::endl; break;
        case t_gt: std::cout << ">" << std::endl; break;
        case t_gte: std::cout << ">=" << std::endl; break;
        case t_lt: std::cout << "<" << std::endl; break;
        case t_lte: std::cout << "<=" << std::endl; break;
        case t_eq: std::cout << "=" << std::endl; break;
        case t_neq: std::cout << "!=" << std::endl; break;
        case t_range: std::cout << ".." << std::endl; break;
        
        case t_id: std::cout << "ID(" << id_val << ")" << std::endl; break;
        case t_string_literal: std::cout << "STR(" << id_val << ")" << std::endl; break;
        case t_char_literal: std::cout << "CHAR(" << i8_val << ")" << std::endl; break;
        case t_int_literal: std::cout << "INT(" << i32_val << ")" << std::endl; break;
        case t_float_literal: std::cout << "FL(" << float_val << ")" << std::endl; break;
        
        default: {}
    }
}

