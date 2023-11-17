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
void Lex::unget(int t) {
    token_stack.push((token)t);
}

//
// Get the next token in the stream
//
int Lex::get_next() {
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
        
        if (c == '#') {
            c = reader.get();
            while (c != '\n' && !reader.eof()) {
                c = reader.get();
            }
            ++line_number;
        }
        
        if (c == '\"') {
            value = "";
            
            c = reader.get();
            while (c != '\"' && !reader.eof()) {
                value += c;
                c = reader.get();     
            }
            
            return t_string_literal;
        }
        
        if (c == '\'') {
            c = reader.get();
            if (c == '\\') {
                c = reader.get();
                if (c == 'n') {
                    c = '\n';
                }
            }
            
            token t = t_char_literal;
            i_value = (char)c;
            value = "";
            value += c;
            
            reader.get();
            return t;
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
            ///LEX_KEYWORD_CHECK
            else if (is_integer()) {
                t = t_int_literal;
                value = buffer;
                i_value = std::stoi(buffer);
            } else if (is_hex()) {
                t = t_int_literal;
                value = buffer;
                i_value = std::stoi(buffer, 0, 16);
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
        ///LEX_SYMBOL_CHECK
        
        default: return false;
    }
    return false;
}

token Lex::get_symbol(char c) {
    switch (c) {
        ///LEX_SYMBOL_RETURN
        default: return t_none;
    }
    return t_none;
}

bool Lex::is_integer() {
    for (char c : buffer) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool Lex::is_hex() {
    if (buffer.length() < 3) return false;
    if (buffer[0] != '0' || buffer[1] != 'x') return false;
    
    for (int i = 2; i<buffer.length(); i++) {
        if (!isxdigit(buffer[i])) return false;
    }
    return true;
}

//
// A debug function for the lexical analyzer
//
void Lex::debug_token(int t) {
    switch (t) {
        case t_none: std::cout << "???" << std::endl; break;
        case t_eof: std::cout << "EOF" << std::endl; break;
        
        ///LEX_KEYWORD_DEBUG
        
        ///LEX_SYMBOL_DEBUG
        
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_string_literal: std::cout << "STR(" << value << ")" << std::endl; break;
        case t_char_literal: std::cout << "CHAR(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "INT(" << value << ")" << std::endl; break;
        case t_float_literal: std::cout << "FL(" << value << ")" << std::endl; break;
        
        default: {}
    }
}

