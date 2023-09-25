//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <fstream>
#include <string>
#include <stack>

// Represents a token
enum TokenType {
    EmptyToken,
    Eof,
    
    // Keywords
    Extern,
    Global,
    Local,
    Ret,
    Alloca,
    Load,
    Store,
    Add,
    Sub,
    SMul,
    SDiv,
    Call,
    Br,
    Beq,
    Bne,
    Bgt,
    Blt,
    Bge,
    Ble,
    LoadStruct,
    StoreStruct,
    GetElementPtr,
    And,
    Or,
    Xor,
    Not,
    
    // Datatype Keywords
    Void,
    I8,
    I16,
    I32,
    I64,
    
    // Literals
    Id,
    String,
    CharL,
    Int32,
    
    // Symbols
    SemiColon,
    Colon,
    Assign,
    LParen,
    RParen,
    LCBrace,
    RCBrace,
    Comma,
    Pointer,
    Mod,
    StrSym
};

struct Token {
    TokenType type;
    std::string id_val;
    char i8_val;
    int i32_val;
    
    Token();
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
};

