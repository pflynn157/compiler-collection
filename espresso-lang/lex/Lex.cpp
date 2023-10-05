//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <cctype>

#include <lex/Lex.hpp>

// The token debug function
Token::Token() {
    type = EmptyToken;
    id_val = "";
    i32_val = 0;
}

// The scanner functions
Scanner::Scanner(std::string input) {
    reader = std::ifstream(input.c_str());
    if (!reader.is_open()) {
        std::cout << "Unknown input file." << std::endl;
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
        token.type = Eof;
        return token;
    }
    
    for (;;) {
        char next = reader.get();
        if (reader.eof()) {
            token.type = Eof;
            break;
        }
        
        rawBuffer += next;
        
        if (next == '#') {
            while (next != '\n' && !reader.eof()) {
                next = reader.get();
                rawBuffer += next;
            }
            ++currentLine;
        }
        
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
            charL.type = CharL;
            
            next = reader.get();
            rawBuffer += next;
            return charL;
        }
        
        if (next == '\"') {
            if (inQuote) {
                Token str;
                str.type = String;
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
            if (buffer.length() == 0) {
                if (isSymbol(next)) {
                    Token sym;
                    sym.type = getSymbol(next);
                    return sym;
                }
                continue;
            }
            
            // Check if we have a symbol
            if (isSymbol(next)) {
                Token sym;
                sym.type = getSymbol(next);
                token_stack.push(sym);
            }
            
            // Now check the buffer
            token.type = getKeyword();
            if (token.type != EmptyToken) {
                buffer = "";
                break;
            }
            
            if (isInt()) {
                token.type = Int32;
                token.i32_val = std::stoi(buffer);
            } else if (isHex()) {
                token.type = Int32;
                token.i32_val = std::stoi(buffer, 0, 16);
            } else {
                token.type = Id;
                token.id_val = buffer;
            }
            
            // Reset everything
            buffer = "";
            break;
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
        case ';': 
        case ':': 
        case '.':
        case '=':
        case '(':
        case ')':
        case '[':
        case ']':
        case ',': 
        case '+': 
        case '-': 
        case '*': 
        case '/':
        case '%':
        case '>':
        case '<':
        case '&':
        case '|':
        case '^':
        case '!': return true;
    }
    return false;
}

TokenType Scanner::getKeyword() {
    if (buffer == "func") return Func;
    else if (buffer == "routine") return Routine;
    else if (buffer == "public") return Public;
    else if (buffer == "protected") return Protected;
    else if (buffer == "private") return Private;
    else if (buffer == "end") return End;
    else if (buffer == "return") return Return;
    else if (buffer == "var") return VarD;
    else if (buffer == "const") return Const;
    else if (buffer == "bool") return Bool;
    else if (buffer == "char") return Char;
    else if (buffer == "byte") return Byte;
    else if (buffer == "ubyte") return UByte;
    else if (buffer == "short") return Short;
    else if (buffer == "ushort") return UShort;
    else if (buffer == "int") return Int;
    else if (buffer == "uint") return UInt;
    else if (buffer == "int64") return Int64;
    else if (buffer == "uint64") return UInt64;
    else if (buffer == "str") return Str;
    else if (buffer == "if") return If;
    else if (buffer == "elif") return Elif;
    else if (buffer == "else") return Else;
    else if (buffer == "while") return While;
    else if (buffer == "is") return Is;
    else if (buffer == "then") return Then;
    else if (buffer == "do") return Do;
    else if (buffer == "break") return Break;
    else if (buffer == "continue") return Continue;
    else if (buffer == "true") return True;
    else if (buffer == "false") return False;
    return EmptyToken;
}

TokenType Scanner::getSymbol(char c) {
    switch (c) {
        case ';': return SemiColon;
        case '(': return LParen;
        case ')': return RParen;
        case '[': return LBracket;
        case ']': return RBracket;
        case ',': return Comma;
        case '+': return Plus;
        case '*': return Mul;
        case '/': return Div;
        case '%': return Mod;
        case '=': return EQ;
        case '&': return And;
        case '|': return Or;
        case '^': return Xor;
        case '.': return Dot;
        
        case ':': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
                return Assign;
            } else {
                reader.unget();
                return Colon;
            }
        } break;
        
        case '>': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
                return GTE;
            } else if (c2 == '>') {
                rawBuffer += c2;
                return Rsh;
            } else {
                reader.unget();
                return GT;
            }
        } break;
        
        case '<': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
                return LTE;
            } else if (c2 == '<') {
                rawBuffer += c2;
                return Lsh;
            } else {
                reader.unget();
                return LT;
            }
        } break;
        
        case '!': {
            char c2 = reader.get();
            if (c2 == '=') {
                rawBuffer += c2;
                return NEQ;
            } else {
                reader.unget();
            }
        } break;
        
        case '-': {
            char c2 = reader.get();
            if (c2 == '>') {
                rawBuffer += c2;
                return Arrow;
            } else {
                reader.unget();
                return Minus;
            }
        } break;
    }
    return EmptyToken;
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

//
// Print the token- debug function
//
void Token::print() {
    switch (type) {
        case EmptyToken: std::cout << "?? "; break;
        case Eof: std::cout << "EOF "; break;
        
        case Func: std::cout << "FUNC "; break;
        case End: std::cout << "END "; break;
        case Return: std::cout << "RETURN "; break;
        case VarD: std::cout << "VAR "; break;
        case Const: std::cout << "CONST "; break;
        case If: std::cout << "IF"; break;
        case Elif: std::cout << "ELIF"; break;
        case Else: std::cout << "ELSE"; break;
        case While: std::cout << "WHILE"; break;
        case Is: std::cout << "IS"; break;
        case Then: std::cout << "THEN"; break;
        case Do: std::cout << "DO"; break;
        case Break: std::cout << "BREAK"; break;
        case Continue: std::cout << "CONTINUE"; break;
        
        case Bool: std::cout << "BOOL"; break;
        case Char: std::cout << "CHAR"; break;
        case Byte: std::cout << "BYTE"; break;
        case UByte: std::cout << "UBYTE"; break;
        case Short: std::cout << "SHORT"; break;
        case UShort: std::cout << "USHORT"; break;
        case Int: std::cout << "INT"; break;
        case UInt: std::cout << "UINT"; break;
        case Int64: std::cout << "INT64"; break;
        case UInt64: std::cout << "UINT64"; break;
        case Str: std::cout << "STR"; break;
        
        case Id: std::cout << "ID "; break;
        case Int32: std::cout << "I32 "; break;
        case True: std::cout << "TRUE "; break;
        case False: std::cout << "FALSE "; break;
        
        case Nl: std::cout << "\\n "; break;
        case SemiColon: std::cout << "; "; break;
        case Colon: std::cout << ": "; break;
        case Assign: std::cout << ":= "; break;
        case LParen: std::cout << "("; break;
        case RParen: std::cout << ")"; break;
        case LBracket: std::cout << "["; break;
        case RBracket: std::cout << "]"; break;
        case Comma: std::cout << ", "; break;
        case Dot: std::cout << ". "; break;
        case Arrow: std::cout << "-> "; break;
        
        case Plus: std::cout << "+ "; break;
        case Minus: std::cout << "- "; break;
        case Mul: std::cout << "* "; break;
        case Div: std::cout << "/ "; break;
        
        case EQ: std::cout << "== "; break;
        case NEQ: std::cout << "!= "; break;
        case GT: std::cout << "> "; break;
        case LT: std::cout << "< "; break;
        case GTE: std::cout << ">= "; break;
        case LTE: std::cout << "<= "; break;
        
        default: {}
    }
    
    std::cout << id_val << " ";
    std::cout << i32_val << " ";
    
    std::cout << std::endl;
}

