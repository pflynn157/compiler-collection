#include <iostream>

#include "lex.hpp"

void Token::print() {
    switch (this->type) {
        /*case EmptyToken: std::cout << "???" << std::endl; break;
        case Eof: std::cout << "EOF" << std::endl; break;
        
        case Extern: std::cout << "extern" << std::endl; break;
        case Func: std::cout << "func" << std::endl; break;
        case Struct: std::cout << "struct" << std::endl; break;
        case End: std::cout << "end" << std::endl; break;
        case Return: std::cout << "return" << std::endl; break;
        case VarD: std::cout << "var" << std::endl; break;
        case Const: std::cout << "const" << std::endl; break;
        case Bool: std::cout << "bool" << std::endl; break;
        case Char: std::cout << "char" << std::endl; break;
        case Str: std::cout << "string" << std::endl; break;
        case I8: std::cout << "i8" << std::endl; break;
        case U8: std::cout << "u8" << std::endl; break;
        case I16: std::cout << "i16" << std::endl; break;
        case U16: std::cout << "u16" << std::endl; break;
        case I32: std::cout << "i32" << std::endl; break;
        case U32: std::cout << "u32" << std::endl; break;
        case I64: std::cout << "i64" << std::endl; break;
        case U64: std::cout << "u64" << std::endl; break;
        case If: std::cout << "if" << std::endl; break;
        case Elif: std::cout << "elif" << std::endl; break;
        case Else: std::cout << "else" << std::endl; break;
        case While: std::cout << "while" << std::endl; break;
        case Is: std::cout << "is" << std::endl; break;
        case Then: std::cout << "then" << std::endl; break;
        case Do: std::cout << "do" << std::endl; break;
        case Break: std::cout << "break" << std::endl; break;
        case Continue: std::cout << "continue" << std::endl; break;
        case Import: std::cout << "import" << std::endl; break;
        case True: std::cout << "true" << std::endl; break;
        case False: std::cout << "false" << std::endl; break;
        case Logical_And: std::cout << "and" << std::endl; break;
        case Logical_Or: std::cout << "or" << std::endl; break;
        case Dot: std::cout << "." << std::endl; break;
        case SemiColon: std::cout << ";" << std::endl; break;
        case Comma: std::cout << "," << std::endl; break;
        case LParen: std::cout << "(" << std::endl; break;
        case RParen: std::cout << ")" << std::endl; break;
        case LBracket: std::cout << "[" << std::endl; break;
        case RBracket: std::cout << "]" << std::endl; break;
        case Plus: std::cout << "+" << std::endl; break;
        case Minus: std::cout << "-" << std::endl; break;
        case Arrow: std::cout << "->" << std::endl; break;
        case Mul: std::cout << "*" << std::endl; break;
        case Div: std::cout << "/" << std::endl; break;
        case Mod: std::cout << "%" << std::endl; break;
        case And: std::cout << "&" << std::endl; break;
        case Or: std::cout << "|" << std::endl; break;
        case Xor: std::cout << "^" << std::endl; break;
        case Colon: std::cout << ":" << std::endl; break;
        case Assign: std::cout << ":=" << std::endl; break;
        case GT: std::cout << ">" << std::endl; break;
        case GTE: std::cout << ">=" << std::endl; break;
        case LT: std::cout << "<" << std::endl; break;
        case LTE: std::cout << "<=" << std::endl; break;
        case EQ: std::cout << "=" << std::endl; break;
        case NEQ: std::cout << "!=" << std::endl; break;
        
        case Id: std::cout << "ID(" << id_val << ")" << std::endl; break;
        case String: std::cout << "STR(" << id_val << ")" << std::endl; break;
        case CharL: std::cout << "CHAR(" << i8_val << ")" << std::endl; break;
        case Int32: std::cout << "INT(" << i32_val << ")" << std::endl; break;
        case FloatL: std::cout << "FL(" << float_val << ")" << std::endl; break;*/
        
        default: {}
    }
}

