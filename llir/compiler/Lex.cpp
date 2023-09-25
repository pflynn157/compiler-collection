//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <cctype>

#include <Lex.hpp>

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
        
        if (next == '#' && !inQuote) {
            std::string comment = "#";
            while (next != '\n' && !reader.eof()) {
                next = reader.get();
                rawBuffer += next;
                comment += next;
            }
            if (comment == "#pragma nocount\n") {
                skipNextLineCount = true;
            } else {
               ++currentLine;
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
            buffer += next;
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
        case '=':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case '*':
        case '%':
        case '$': return true;
    }
    return false;
}

TokenType Scanner::getKeyword() {
    if (buffer == "extern") return Extern;
    else if (buffer == "global") return Global;
    else if (buffer == "local") return Local;
    else if (buffer == "void") return Void;
    else if (buffer == "i8") return I8;
    else if (buffer == "i16") return I16;
    else if (buffer == "i32") return I32;
    else if (buffer == "i64") return I64;
    else if (buffer == "ret") return Ret;
    else if (buffer == "alloca") return Alloca;
    else if (buffer == "load") return Load;
    else if (buffer == "store") return Store;
    else if (buffer == "add") return Add;
    else if (buffer == "sub") return Sub;
    else if (buffer == "smul") return SMul;
    else if (buffer == "sdiv") return SDiv;
    else if (buffer == "call") return Call;
    else if (buffer == "br") return Br;
    else if (buffer == "beq") return Beq;
    else if (buffer == "bne") return Bne;
    else if (buffer == "bgt") return Bgt;
    else if (buffer == "blt") return Blt;
    else if (buffer == "bge") return Bge;
    else if (buffer == "ble") return Ble;
    else if (buffer == "load.struct") return LoadStruct;
    else if (buffer == "store.struct") return StoreStruct;
    else if (buffer == "getelementptr") return GetElementPtr;
    else if (buffer == "and") return And;
    else if (buffer == "or") return Or;
    else if (buffer == "xor") return Xor;
    else if (buffer == "not") return Not;
    return EmptyToken;
}

TokenType Scanner::getSymbol(char c) {
    switch (c) {
        case ';': return SemiColon;
        case ':': return Colon;
        case '=': return Assign;
        case '(': return LParen;
        case ')': return RParen;
        case '{': return LCBrace;
        case '}': return RCBrace;
        case ',': return Comma;
        case '*': return Pointer;
        case '%': return Mod;
        case '$': return StrSym;
    }
    return EmptyToken;
}

bool Scanner::isInt() {
    /*for (char c : buffer) {
        if (!isdigit(c)) return false;
    }*/
    for (int i = 0; i<buffer.length(); i++) {
        if (buffer[i] == '-' && i == 0) continue;
        if (!isdigit(buffer[i])) return false;
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
