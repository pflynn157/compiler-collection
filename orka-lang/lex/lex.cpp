#include <iostream>
#include <cctype>

#include "lex.hpp"

// The token debug function
Token::Token() {
    type = t_none;
    id_val = "";
    i32_val = 0;
}

// The scanner functions
Scanner::Scanner(std::string input) {
    reader = std::ifstream(input.c_str());
    if (!reader.is_open()) {
        std::cout << "Unknown input file." << std::endl;
        std::cout << "--> " << input << std::endl;
        error = true;
    }
}

Scanner::~Scanner() {
    reader.close();
}

void Scanner::rewind(Token token) {
    token_stack.push(token);
}

// The main scanning function
Token Scanner::getNext() {
    if (token_stack.size() > 0) {
        Token top = token_stack.top();
        token_stack.pop();
        return top;
    }

    Token token;
    if (reader.eof()) {
        token.type = t_eof;
        return token;
    }
    
    for (;;) {
        char next = reader.get();
        if (reader.eof()) {
            token.type = t_eof;
            break;
        }
        
        rawBuffer += next;
        
        if (next == '#') {
            while (next != '\n' && !reader.eof()) {
                next = reader.get();
                rawBuffer += next;
            }
            continue;
        }
        
        
        
        // TODO: This needs some kind of error handleing
        if (next == '\'') {
            char c = reader.get();
            rawBuffer += c;
            if (c == '\\') {
                c = reader.get();
                if (c == 'n') {
                    c = '\n';
                    rawBuffer += c;
                }
            }
        
            Token charL;
            charL.i8_val = c;
            charL.type = t_char_literal;
            
            next = reader.get();
            rawBuffer += next;
            return charL;
        }
        
        if (next == '\"') {
            if (inQuote) {
                Token str;
                str.type = t_string_literal;
                str.id_val = buffer;
                
                buffer = "";
                inQuote = false;
                return str;
            } else {
                inQuote = true;
                continue;
            }
        }
        
        if (inQuote) {
            if (next == '\\') {
                next = reader.get();
                rawBuffer += next;
                switch (next) {
                    case 'n': buffer += '\n'; break;
                    case 't': buffer += '\t'; break;
                    default: buffer += '\\' + next;
                }
            } else {
                buffer += next;
            }
            continue;
        }
        
        if (next == ' ' || next == '\n' || isSymbol(next)) {
            if (next == '\n') {
                if (skipNextLineCount) skipNextLineCount = false;
                else ++currentLine;
            }
        
            if (buffer.length() == 0) {
                if (isSymbol(next)) {
                    Token sym;
                    sym.type = getSymbol(next);
                    return sym;
                }
                continue;
            }
            
            // Check if we have a symbol
            // Here, we also check to see if we have a floating point
            if (next == '.') {
                if (isInt()) {
                    buffer += ".";
                    continue;
                } else {
                    Token sym;
                    sym.type = getSymbol(next);
                    token_stack.push(sym);
                }
            } else if (isSymbol(next)) {
                Token sym;
                sym.type = getSymbol(next);
                token_stack.push(sym);
            }
            
            // Now check the buffer
            token.type = getKeyword();
            if (token.type != t_none) {
                buffer = "";
                break;
            }
            
            if (isInt()) {
                token.type = t_int_literal;
                token.i32_val = std::stoi(buffer);
            } else if (isHex()) {
                token.type = t_int_literal;
                token.i32_val = std::stoi(buffer, 0, 16);
            } else if (isFloat()) {
                token.type = t_float_literal;
                token.float_val = std::stod(buffer);
            } else {
                token.type = t_id;
                token.id_val = buffer;
            }
            
            // Reset everything
            buffer = "";
            return token;
        } else {
            buffer += next;
        }
    }
    
    return token;
}

std::string Scanner::getRawBuffer() {
    std::string ret = rawBuffer;
    rawBuffer = "";
    return ret;
}

bool Scanner::isSymbol(char c) {
    switch (c) {
        //case ';':
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

TokenType Scanner::getKeyword() {
    //if (buffer == "extern") return Extern;
    if (buffer == "extern") return t_extern;
    else if (buffer == "func") return t_func;
    else if (buffer == "struct") return t_struct;
    else if (buffer == "end") return t_end;
    else if (buffer == "return") return t_return;
    else if (buffer == "var") return t_var;
    else if (buffer == "const") return t_const;
    else if (buffer == "bool") return t_bool;
    else if (buffer == "char") return t_char;
    else if (buffer == "str") return t_string;
    else if (buffer == "byte") return t_i8;
    else if (buffer == "ubyte") return t_u8;
    else if (buffer == "short") return t_i16;
    else if (buffer == "ushort") return t_u16;
    else if (buffer == "int") return t_i32;
    else if (buffer == "uint") return t_u32;
    else if (buffer == "int64") return t_i64;
    else if (buffer == "uint64") return t_u64;
    else if (buffer == "if") return t_if;
    else if (buffer == "elif") return t_elif;
    else if (buffer == "else") return t_else;
    else if (buffer == "while") return t_while;
    else if (buffer == "is") return t_is;
    else if (buffer == "then") return t_then;
    else if (buffer == "do") return t_do;
    else if (buffer == "break") return t_break;
    else if (buffer == "continue") return t_continue;
    else if (buffer == "import") return t_import;
    else if (buffer == "true") return t_true;
    else if (buffer == "false") return t_false;
    else if (buffer == "and") return t_lgand;
    else if (buffer == "or") return t_lgor;
    return t_none;
}

TokenType Scanner::getSymbol(char c) {
    switch (c) {
        case '.': return t_dot;
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
                rawBuffer += c2;
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
                rawBuffer += c2;
                return t_assign;
            } else {
                reader.unget();
                return t_colon;
            }
        } break;
        case '>': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
                return t_gte;
            } else {
                reader.unget();
                return t_gt;
            }
        } break;
        case '<': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
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
                rawBuffer += c2;
                return t_neq;
            } else {
                reader.unget();
            }
        } break;
        default: return t_none;
    }
    return t_none;
}

bool Scanner::isInt() {
    for (char c : buffer) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool Scanner::isHex() {
    if (buffer.length() < 3) return false;
    if (buffer[0] != '0' || buffer[1] != 'x') return false;
    
    for (int i = 2; i<buffer.length(); i++) {
        if (!isxdigit(buffer[i])) return false;
    }
    return true;
}

bool Scanner::isFloat() {
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

