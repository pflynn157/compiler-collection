#include <iostream>
#include <cctype>

#include "lex.hpp"

//
// Setups the lexical analyzer
//
Lex::Lex(std::string input) {
    reader = std::ifstream(input);
    if (!reader.is_open()) {
        // TODO
    }
}

//
// Ungets the token from the stream
//
void Lex::unget(token t) {
    token_stack.push(t);
}

//
// Get the next token in the stream
//
token Lex::get_next() {
    if (!token_stack.empty()) {
        auto t = token_stack.top();
        token_stack.pop();
        return t;
    }

    if (reader.eof()) {
        return t_eof;
    }
    
    while (!reader.eof()) {
        char c = reader.get();
        
        if (c == '\"') {
            value = "";
            
            c = reader.get();
            while (c != '\"' && !reader.eof()) {
                value += c;
                c = reader.get();     
            }
            
            return t_string_literal;
        }
        
        if (c == ' ' || c == '\n' || is_symbol(c)) {
            if (c == '\n') ++line_number;
        
            if (is_symbol(c)) {
                token sym = get_symbol(c);
                if (buffer == "") {
                    return sym;
                }
                token_stack.push(sym);
            }
            
            if (buffer == "") continue;
            
            token t = t_none;
            if (buffer == "module") t = t_module;
            else if (buffer == "func") t = t_func;
            else if (buffer == "of") t = t_of;
            else if (buffer == "is") t = t_is;
            else if (buffer == "return") t = t_return;
            else if (buffer == "end") t = t_end;
            else if (buffer == "scalar") t = t_scalar;
            else if (buffer == "void") t = t_void;
            else if (buffer == "int") t = t_int;
            else if (is_integer()) {
                t = t_int_literal;
                value = buffer;
            } else {
                t = t_id;
                value = buffer;
            }
            
            buffer = "";
            return t;
        } else {
            buffer += c;
        }
    }
    
    return t_eof;
}

bool Lex::is_symbol(char c) {
    switch (c) {
        case '.':
        case '@':
        case ':':
        case ',':
        case '(': case ')':
            return true;
        
        default: {}
    }
    
    return false;
}

token Lex::get_symbol(char c) {
    switch (c) {
        case '.': return t_period;
        case '(': return t_lparen;
        case ')': return t_rparen;
        case '@': return t_annot;
        case ',': return t_comma;
        
        case ':': {
            char c2 = reader.get();
            if (c2 == '=') {
                return t_assign;
            }
            reader.unget();
            return t_colon;
        }
        
        default: {}
    }
    
    return t_none;
}

bool Lex::is_integer() {
    for (char c : buffer) {
        if (!isdigit(c)) return false;
    }
    return true;
}

//
// A debug function for the lexical analyzer
//
void Lex::debug_token(token t) {
    switch (t) {
        case t_eof: std::cout << "EOF" << std::endl; break;
    
        case t_module: std::cout << "MODULE" << std::endl; break;
        case t_func: std::cout << "FUNC" << std::endl; break;
        case t_of: std::cout << "OF" << std::endl; break;
        case t_is: std::cout << "IS" << std::endl; break;
        case t_return: std::cout << "RETURN" << std::endl; break;
        case t_end: std::cout << "END" << std::endl; break;
        case t_scalar: std::cout << "SCALAR" << std::endl; break;
        
        case t_void: std::cout << "VOID" << std::endl; break;
        case t_int: std::cout << "INT" << std::endl; break;
        
        case t_period: std::cout << "." << std::endl; break;
        case t_lparen: std::cout << "(" << std::endl; break;
        case t_rparen: std::cout << ")" << std::endl; break;
        case t_annot: std::cout << "@" << std::endl; break;
        case t_colon: std::cout << ":" << std::endl; break;
        case t_assign: std::cout << ":=" << std::endl; break;
        
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "INT(" << value << ")" << std::endl; break;
        case t_string_literal: std::cout << "STRING(" << value << ")" << std::endl; break;
        
        default: std::cout << "???" << std::endl;
    }
}

