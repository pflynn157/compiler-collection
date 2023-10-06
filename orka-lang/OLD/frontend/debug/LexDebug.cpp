//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <lex/Lex.hpp>

void Token::print() {
    switch (type) {
        case EmptyToken: std::cout << "?? "; break;
        case Eof: std::cout << "EOF "; break;
        
        case Extern: std::cout << "EXTERN "; break;
        case Func: std::cout << "FUNC "; break;
        case Enum: std::cout << "ENUM "; break;
        case Struct: std::cout << "STRUCT "; break;
        case End: std::cout << "END "; break;
        case Return: std::cout << "RETURN "; break;
        case VarD: std::cout << "VAR "; break;
        case Const: std::cout << "CONST "; break;
        case If: std::cout << "IF"; break;
        case Elif: std::cout << "ELIF"; break;
        case Else: std::cout << "ELSE"; break;
        case While: std::cout << "WHILE"; break;
        case For: std::cout << "FOR"; break;
        case ForAll: std::cout << "FORALL"; break;
        case Repeat: std::cout << "REPEAT"; break;
        case Is: std::cout << "IS"; break;
        case Then: std::cout << "THEN"; break;
        case Do: std::cout << "DO"; break;
        case Break: std::cout << "BREAK"; break;
        case Continue: std::cout << "CONTINUE"; break;
        case In: std::cout << "IN"; break;
        case Sizeof: std::cout << "SIZEOF"; break;
        case Import: std::cout << "IMPORT"; break;
        case Step: std::cout << "STEP"; break;
        
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
        case Range: std::cout << ".. "; break;
        case Arrow: std::cout << "-> "; break;
        case Scope: std::cout << ":: "; break;
        
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

