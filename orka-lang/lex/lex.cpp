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
        if (!reader.eof()) raw_buffer += c;
        
        if (c == '#') {
            c = reader.get();
            raw_buffer += c;
            while (c != '\n' && !reader.eof()) {
                c = reader.get();
                raw_buffer += c;
            }
            ++line_number;
        }
        
        if (c == '\"') {
            value = "";
            
            c = reader.get();
            raw_buffer += c;
            while (c != '\"' && !reader.eof()) {
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
			if (buffer == "extern") t = t_extern;
			else if (buffer == "func") t = t_func;
			else if (buffer == "struct") t = t_struct;
			else if (buffer == "end") t = t_end;
			else if (buffer == "return") t = t_return;
			else if (buffer == "var") t = t_var;
			else if (buffer == "const") t = t_const;
			else if (buffer == "bool") t = t_bool;
			else if (buffer == "char") t = t_char;
			else if (buffer == "str") t = t_string;
			else if (buffer == "byte") t = t_i8;
			else if (buffer == "ubyte") t = t_u8;
			else if (buffer == "short") t = t_i16;
			else if (buffer == "ushort") t = t_u16;
			else if (buffer == "int") t = t_i32;
			else if (buffer == "uint") t = t_u32;
			else if (buffer == "int64") t = t_i64;
			else if (buffer == "uint64") t = t_u64;
			else if (buffer == "if") t = t_if;
			else if (buffer == "elif") t = t_elif;
			else if (buffer == "else") t = t_else;
			else if (buffer == "while") t = t_while;
			else if (buffer == "is") t = t_is;
			else if (buffer == "then") t = t_then;
			else if (buffer == "do") t = t_do;
			else if (buffer == "break") t = t_break;
			else if (buffer == "continue") t = t_continue;
			else if (buffer == "import") t = t_import;
			else if (buffer == "true") t = t_true;
			else if (buffer == "false") t = t_false;
			else if (buffer == "and") t = t_lgand;
			else if (buffer == "or") t = t_lgor;
			else if (buffer == "sizeof") t = t_sizeof;
			else if (buffer == "for") t = t_for;
			else if (buffer == "forall") t = t_forall;
			else if (buffer == "in") t = t_in;
			else if (buffer == "step") t = t_step;
			else if (buffer == "repeat") t = t_repeat;
			else if (buffer == "enum") t = t_enum;
			else if (buffer == "class") t = t_class;
			else if (buffer == "extends") t = t_extends;
			else if (buffer == "float") t = t_float;
			else if (buffer == "double") t = t_double;
            else if (is_integer()) {
                t = t_int_literal;
                value = buffer;
                i_value = std::stoi(buffer);
            } else if (is_hex()) {
                t = t_int_literal;
                value = buffer;
                i_value = std::stoi(buffer, 0, 16);
            } else if (is_float()) {
                t = t_float_literal;
                value = buffer;
                f_value = std::stod(buffer);
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

std::string Lex::get_raw_buffer() {
    std::string ret = raw_buffer;
    raw_buffer = "";
    return ret;
}

bool Lex::is_symbol(char c) {
    switch (c) {
		case '.': return true;
		case ';': return true;
		case ',': return true;
		case '(': return true;
		case ')': return true;
		case '[': return true;
		case ']': return true;
		case '+': return true;
		case '-': return true;
		case '*': return true;
		case '/': return true;
		case '%': return true;
		case '&': return true;
		case '|': return true;
		case '^': return true;
		case ':': return true;
		case '>': return true;
		case '<': return true;
		case '=': return true;
		case '!': return true;
        
        default: return false;
    }
    return false;
}

token Lex::get_symbol(char c) {
    switch (c) {
		case '.': {
			char c2 = reader.get();
			if (c2 == '.') {
				raw_buffer += c2;
				return t_range;
			} else {
				reader.unget();
				return t_dot;
			}
		} break;
		case ';': return t_semicolon;
		case ',': return t_comma;
		case '(': return t_lparen;
		case ')': return t_rparen;
		case '[': return t_lbracket;
		case ']': return t_rbracket;
		case '+': return t_plus;
		case '-': {
			char c2 = reader.get();
			if (c2 == '>') {
				raw_buffer += c2;
				return t_arrow;
			} else {
				reader.unget();
				return t_minus;
			}
		} break;
		case '*': return t_mul;
		case '/': return t_div;
		case '%': return t_mod;
		case '&': return t_and;
		case '|': return t_or;
		case '^': return t_xor;
		case ':': {
			char c2 = reader.get();
			if (c2 == '=') {
				raw_buffer += c2;
				return t_assign;
			} else 			if (c2 == ':') {
				raw_buffer += c2;
				return t_scope;
			} else {
				reader.unget();
				return t_colon;
			}
		} break;
		case '>': {
			char c2 = reader.get();
			if (c2 == '=') {
				raw_buffer += c2;
				return t_gte;
			} else {
				reader.unget();
				return t_gt;
			}
		} break;
		case '<': {
			char c2 = reader.get();
			if (c2 == '=') {
				raw_buffer += c2;
				return t_lte;
			} else {
				reader.unget();
				return t_lt;
			}
		} break;
		case '=': return t_eq;
		case '!': {
			char c2 = reader.get();
			if (c2 == '=') {
				raw_buffer += c2;
				return t_neq;
			} else {
				reader.unget();
			}
		} break;
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

bool Lex::is_float() {
    bool foundDot = false;
    for (char c : buffer) {
        if (c == '.') {
            if (foundDot) return false;
            foundDot = true;
        } else if (!isdigit(c)) {
            return false;
        }
    }
    if (!foundDot) return false;
    return true;
}

//
// A debug function for the lexical analyzer
//
void Lex::debug_token(int t) {
    switch (t) {
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
		case t_string: std::cout << "str" << std::endl; break;
		case t_i8: std::cout << "byte" << std::endl; break;
		case t_u8: std::cout << "ubyte" << std::endl; break;
		case t_i16: std::cout << "short" << std::endl; break;
		case t_u16: std::cout << "ushort" << std::endl; break;
		case t_i32: std::cout << "int" << std::endl; break;
		case t_u32: std::cout << "uint" << std::endl; break;
		case t_i64: std::cout << "int64" << std::endl; break;
		case t_u64: std::cout << "uint64" << std::endl; break;
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
		case t_forall: std::cout << "forall" << std::endl; break;
		case t_in: std::cout << "in" << std::endl; break;
		case t_step: std::cout << "step" << std::endl; break;
		case t_repeat: std::cout << "repeat" << std::endl; break;
		case t_enum: std::cout << "enum" << std::endl; break;
		case t_class: std::cout << "class" << std::endl; break;
		case t_extends: std::cout << "extends" << std::endl; break;
		case t_float: std::cout << "float" << std::endl; break;
		case t_double: std::cout << "double" << std::endl; break;
        
		case t_dot: std::cout << "." << std::endl; break;
		case t_semicolon: std::cout << ";" << std::endl; break;
		case t_comma: std::cout << "," << std::endl; break;
		case t_lparen: std::cout << "(" << std::endl; break;
		case t_rparen: std::cout << ")" << std::endl; break;
		case t_lbracket: std::cout << "[" << std::endl; break;
		case t_rbracket: std::cout << "]" << std::endl; break;
		case t_plus: std::cout << "+" << std::endl; break;
		case t_minus: std::cout << "-" << std::endl; break;
		case t_mul: std::cout << "*" << std::endl; break;
		case t_div: std::cout << "/" << std::endl; break;
		case t_mod: std::cout << "%" << std::endl; break;
		case t_and: std::cout << "&" << std::endl; break;
		case t_or: std::cout << "|" << std::endl; break;
		case t_xor: std::cout << "^" << std::endl; break;
		case t_colon: std::cout << ":" << std::endl; break;
		case t_gt: std::cout << ">" << std::endl; break;
		case t_gte: std::cout << ">=" << std::endl; break;
		case t_lt: std::cout << "<" << std::endl; break;
		case t_lte: std::cout << "<=" << std::endl; break;
		case t_eq: std::cout << "=" << std::endl; break;
		case t_neq: std::cout << "!=" << std::endl; break;
		case t_assign: std::cout << ":=" << std::endl; break;
		case t_arrow: std::cout << "->" << std::endl; break;
		case t_range: std::cout << ".." << std::endl; break;
		case t_scope: std::cout << "::" << std::endl; break;
        
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_string_literal: std::cout << "STR(" << value << ")" << std::endl; break;
        case t_char_literal: std::cout << "CHAR(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "INT(" << value << ")" << std::endl; break;
        case t_float_literal: std::cout << "FL(" << value << ")" << std::endl; break;
        
        default: {}
    }
}

