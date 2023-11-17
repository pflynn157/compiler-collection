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
			if (buffer == "module") t = t_module;
			else if (buffer == "func") t = t_func;
			else if (buffer == "of") t = t_of;
			else if (buffer == "is") t = t_is;
			else if (buffer == "return") t = t_return;
			else if (buffer == "end") t = t_end;
			else if (buffer == "scalar") t = t_scalar;
			else if (buffer == "iter") t = t_iter;
			else if (buffer == "do") t = t_do;
			else if (buffer == "by") t = t_by;
			else if (buffer == "void") t = t_void;
			else if (buffer == "int") t = t_int;
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
		case '.': return true;
		case '(': return true;
		case ')': return true;
		case '@': return true;
		case ':': return true;
		case ',': return true;
        
        default: return false;
    }
    return false;
}

token Lex::get_symbol(char c) {
    switch (c) {
		case '.': return t_period;
		case '(': return t_lparen;
		case ')': return t_rparen;
		case '@': return t_annot;
		case ':': {
			char c2 = reader.get();
			if (c2 == '=') {
				return t_assign;
			} else {
				reader.unget();
				return t_colon;
			}
		} break;
		case ',': return t_comma;
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
        
		case t_module: std::cout << "module" << std::endl; break;
		case t_func: std::cout << "func" << std::endl; break;
		case t_of: std::cout << "of" << std::endl; break;
		case t_is: std::cout << "is" << std::endl; break;
		case t_return: std::cout << "return" << std::endl; break;
		case t_end: std::cout << "end" << std::endl; break;
		case t_scalar: std::cout << "scalar" << std::endl; break;
		case t_iter: std::cout << "iter" << std::endl; break;
		case t_do: std::cout << "do" << std::endl; break;
		case t_by: std::cout << "by" << std::endl; break;
		case t_void: std::cout << "void" << std::endl; break;
		case t_int: std::cout << "int" << std::endl; break;
        
		case t_period: std::cout << "." << std::endl; break;
		case t_lparen: std::cout << "(" << std::endl; break;
		case t_rparen: std::cout << ")" << std::endl; break;
		case t_annot: std::cout << "@" << std::endl; break;
		case t_colon: std::cout << ":" << std::endl; break;
		case t_assign: std::cout << ":=" << std::endl; break;
		case t_comma: std::cout << "," << std::endl; break;
        
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_string_literal: std::cout << "STR(" << value << ")" << std::endl; break;
        case t_char_literal: std::cout << "CHAR(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "INT(" << value << ")" << std::endl; break;
        case t_float_literal: std::cout << "FL(" << value << ")" << std::endl; break;
        
        default: {}
    }
}

