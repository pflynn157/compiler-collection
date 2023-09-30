//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
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
    Func,
    Routine,
    Public,
    Protected,
    Private,
    Enum,
    End,
    Return,
    VarD,
    Const,
    If,
    Elif,
    Else,
    While,
    Repeat,
    For,
    ForAll,
    Is,
    Then,
    Do,
    Break,
    Continue,
    In,
    Sizeof,
    Import,
    Step,
    
    // Datatype Keywords
    Bool,
    Char,
    Byte,
    UByte,
    Short,
    UShort,
    Int,
    UInt,
    Int64,
    UInt64,
    Str,
    
    // Literals
    Id,
    String,
    CharL,
    Int32,
    True,
    False,
    
    // Symbols
    Nl,
    SemiColon,
    Colon,
    Assign,
    LParen,
    RParen,
    LBracket,
    RBracket,
    Comma,
    Dot,
    Range,
    Arrow,
    Scope,
    
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    
    And,
    Or,
    Xor,
    Lsh,
    Rsh,
    
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
};

struct Token {
    TokenType type;
    std::string id_val;
    char i8_val;
    int i32_val;
    
    Token();
    void print();
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
    
    // Functions
    bool isSymbol(char c);
    TokenType getKeyword();
    TokenType getSymbol(char c);
    bool isInt();
    bool isHex();
};

