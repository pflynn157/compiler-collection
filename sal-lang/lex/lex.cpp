//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <cctype>

#include "lex.hpp"

//
// Initialize the lexer
//
Lex::Lex(std::string input) {
    reader = std::ifstream(input);
}

//
// Get the next token in the stream
//
token Lex::get_next() {
    if (!stack.empty()) {
        auto t = stack.top();
        stack.pop();
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
            while (c != '\"') {
                if (c == '\\') {
                    c = reader.get();
                    if (c == 'n') {
                        value += '\n';
                    } else {
                        value += '\\';
                        value += c;
                    }
                } else {
                    value += c;
                }
                c = reader.get();
            }
            
            buffer = "";
            return t_string_literal;
        }
        
        if (c == ' ' || c == '\n' || is_symbol(c)) {
            if (is_symbol(c)) {
                token t = get_symbol(c);
                if (buffer.length() > 0) {
                    stack.push(t);
                } else {
                    return t;
                }
            }
            
            if (buffer.length() == 0) continue;
            
            token t = t_none;
            if (buffer == "func") t = t_func;
            else if (buffer == "return") t = t_return;
            else if (buffer == "of") t = t_of;
            else if (buffer == "end") t = t_end;
            else if (buffer == "extern") t = t_extern;
            else if (buffer == "def") t = t_def;
            else if (buffer == "any") t = t_any;
            else if (buffer == "string") t = t_string;
            else if (is_int()) {
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

//
// Check and see if a character is a symbol or part of one
//
bool Lex::is_symbol(char c) {
    switch (c) {
        case '.':
        case '{':
        case '}':
        case '#':
        case ':':
        case ',': return true;
        
        default: {}
    }
    
    return false;
}

//
// Returns the symbol for the equivalent token
//
token Lex::get_symbol(char c) {
    switch (c) {
        case '.': return t_dot;
        case '{': return t_lcbrace;
        case '}': return t_rcbrace;
        case '#': return t_numsym;
        case ':': return t_colon;
        case ',': return t_comma;
        
        default: {}
    }
    
    return t_none;
}

//
// Check to see if a given literal is an integer
//
bool Lex::is_int() {
    for (char c : buffer) {
        if (!isdigit(c)) return false;
    }
    return true;
}

//
// Our lexical debug function
//
void Lex::print(token t) {
    switch (t) {
        case t_eof: std::cout << "EOF" << std::endl; break;
    
        // Keywords
        case t_func: std::cout << "FUNC" << std::endl; break;
        case t_return: std::cout << "RETURN" << std::endl; break;
        case t_of: std::cout << "OF" << std::endl; break;
        case t_end: std::cout << "END" << std::endl; break;
        case t_def: std::cout << "DEF" << std::endl; break;
        case t_extern: std::cout << "EXTERN" << std::endl; break;
        case t_any: std::cout << "ANY" << std::endl; break;
        
        case t_string: std::cout << "STRING" << std::endl; break;

        // Symbols
        case t_dot: std::cout << "." << std::endl; break;
        case t_lcbrace: std::cout << "{" << std::endl; break;
        case t_rcbrace: std::cout << "}" << std::endl; break;
        case t_numsym: std::cout << "#" << std::endl; break;
        case t_colon: std::cout << ":" << std::endl; break;
        case t_comma: std::cout << "," << std::endl; break;

        // Literals
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "VAL(" << value << ")" << std::endl; break;
        case t_string_literal: std::cout << "STR(" << value << ")" << std::endl; break;
        
        default: std::cout << "???" << std::endl;
    }
}

