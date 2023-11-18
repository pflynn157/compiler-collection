//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include <parser/Parser.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
    
    // Add the built-in functions
    //string malloc(string)
    tree->block->funcs.push_back("malloc");
    std::shared_ptr<AstExternFunction> FT1 = std::make_shared<AstExternFunction>("malloc");
    FT1->addArgument(Var(AstBuilder::buildInt32Type(), "size"));
    FT1->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT1);
    
    //print(string)
    tree->block->funcs.push_back("print");
    std::shared_ptr<AstExternFunction> FT3 = std::make_shared<AstExternFunction>("print");
    FT3->varargs = true;
    FT3->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT3->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(FT3);
    
    //i32 strlen(string)
    tree->block->funcs.push_back("strlen");
    std::shared_ptr<AstExternFunction> FT4 = std::make_shared<AstExternFunction>("strlen");
    FT4->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT4->data_type = AstBuilder::buildInt32Type();
    tree->addGlobalStatement(FT4);
    
    //i32 stringcmp(string, string)
    tree->block->funcs.push_back("stringcmp");
    std::shared_ptr<AstExternFunction> FT5 = std::make_shared<AstExternFunction>("stringcmp");
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->data_type = AstBuilder::buildInt32Type();
    tree->addGlobalStatement(FT5);
    
    //string strcat_str(string, string)
    tree->block->funcs.push_back("strcat_str");
    std::shared_ptr<AstExternFunction> FT6 = std::make_shared<AstExternFunction>("strcat_str");
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT6);
    
    //string strcat_char(string, char)
    tree->block->funcs.push_back("strcat_char");
    std::shared_ptr<AstExternFunction> FT7 = std::make_shared<AstExternFunction>("strcat_char");
    FT7->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT7->addArgument(Var(AstBuilder::buildCharType(), "c"));
    FT7->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT7);
}

Parser::~Parser() {
}

bool Parser::parse() {
    int tk;
    do {
        tk = lex->get_next();
        bool code = true;
        
        switch (tk) {
            case t_extern:
            case t_func: {
                code = buildFunction(tk);
            } break;
            
            case t_const: code = buildConst(tree->block, true); break;
            case t_struct: code = buildStruct(); break;
            
            case t_eof: break;
            
            default: {
                syntax->addError(lex->line_number, "Invalid token in global scope.");
                lex->debug_token(tk);
                code = false;
            }
        }
        
        if (!code) break;
    } while (tk != t_eof);
    
    // Check for errors, and print if so
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    syntax->printWarnings();
    return true;
}

// Builds a statement block
bool Parser::buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent) {
    int tk = lex->get_next();
    while (tk != t_end && tk != t_eof) {
        bool code = true;
        bool end = false;
        
        switch (tk) {
            case t_var: code = buildVariableDec(block); break;
            case t_struct: code = buildStructDec(block); break;
            case t_const: code = buildConst(block, false); break;
            
            case t_id: {
                int idtoken = tk;
                std::string value = lex->value;
                tk = lex->get_next();
                
                if (tk == t_assign || tk == t_lbracket || tk == t_dot) {
                    lex->unget(tk);
                    lex->unget(idtoken);
                    code = buildVariableAssign(block, value);
                } else if (tk == t_lparen) {
                    code = buildFunctionCallStmt(block, value);
                } else {
                    syntax->addError(lex->line_number, "Invalid use of identifier.");
                    return false;
                }
            } break;
            
            case t_return: code = buildReturn(block); break;
            
            // Handle conditionals
            case t_if: code = buildConditional(block); break;
            case t_elif: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                code = buildConditional(condParent->false_block);
                end = true;
            } break;
            case t_else: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                buildBlock(condParent->false_block);
                end = true;
            } break;
            
            // Handle loops
            case t_while: code = buildWhile(block); break;
            case t_break: code = buildLoopCtrl(block, true); break;
            case t_continue: code = buildLoopCtrl(block, false); break;
            
            default: {
                syntax->addError(lex->line_number, "Invalid token in block.");
                return false;
            }
        }
        
        if (end) break;
        if (!code) return false;
        tk = lex->get_next();
    }
    
    return true;
}

// The debug function for the scanner
void Parser::debugScanner() {
    std::cout << "Debugging scanner..." << std::endl;
    
    int t;
    do {
        t = lex->get_next();
        lex->debug_token(t);
    } while (t != t_eof);
}

//
// Builds a data type from the token stream
//
std::shared_ptr<AstDataType> Parser::buildDataType(bool checkBrackets) {
    int tk = lex->get_next();
    std::shared_ptr<AstDataType> dataType = nullptr;
    
    switch (tk) {
        case t_bool: dataType = AstBuilder::buildBoolType(); break;
        case t_char: dataType = AstBuilder::buildCharType(); break;
        case t_i8: dataType = AstBuilder::buildInt8Type(); break;
        case t_u8: dataType = AstBuilder::buildInt8Type(true); break;
        case t_i16: dataType = AstBuilder::buildInt16Type(); break;
        case t_u16: dataType = AstBuilder::buildInt16Type(true); break;
        case t_i32: dataType = AstBuilder::buildInt32Type(); break;
        case t_u32: dataType = AstBuilder::buildInt32Type(true); break;
        case t_i64: dataType = AstBuilder::buildInt64Type(); break;
        case t_u64: dataType = AstBuilder::buildInt64Type(true); break;
        case t_string: dataType = AstBuilder::buildStringType(); break;
        
        case t_id: {
            bool isStruct = false;
            for (auto s : tree->structs) {
                if (s->name == lex->value) {
                    isStruct = true;
                    break;
                }
            }
                
            if (isStruct) {
                dataType = AstBuilder::buildStructType(lex->value);
            }
        } break;
        
        default: {}
    }

    if (checkBrackets) {
        tk = lex->get_next();
        if (tk == t_lbracket) {
            consume_token(t_rbracket, "Invalid pointer type.");
            dataType = AstBuilder::buildPointerType(dataType);
        } else {
            lex->unget(tk);
        }
    }
    
    return dataType;
}

//
// A helper function for lexical analysis
//
void Parser::consume_token(token expected, std::string msg) {
    int t = lex->get_next();
    if (t != expected) {
        syntax->addError(lex->line_number, msg);
    }
}

