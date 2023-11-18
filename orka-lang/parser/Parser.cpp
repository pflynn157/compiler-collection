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
    
    //println(string)
    tree->block->funcs.push_back("println");
    std::shared_ptr<AstExternFunction> FT2 = std::make_shared<AstExternFunction>("println");
    FT2->varargs = true;
    FT2->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT2->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(FT2);
    
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
    
    // Create structures for the internal arrays
    // Int8
    auto int8ArrayStruct = std::make_shared<AstStruct>("__int8_array");
    int8ArrayStruct->addItem(Var(AstBuilder::buildPointerType(AstBuilder::buildInt8Type()), "ptr"), nullptr);
    int8ArrayStruct->addItem(Var(AstBuilder::buildInt32Type(), "size"), std::make_shared<AstInt>(0));
    tree->addStruct(int8ArrayStruct);
    
    // Int16
    auto int16ArrayStruct = std::make_shared<AstStruct>("__int16_array");
    int16ArrayStruct->addItem(Var(AstBuilder::buildPointerType(AstBuilder::buildInt16Type()), "ptr"), nullptr);
    int16ArrayStruct->addItem(Var(AstBuilder::buildInt32Type(), "size"), std::make_shared<AstInt>(0));
    tree->addStruct(int16ArrayStruct);
    
    // Int32
    auto int32ArrayStruct = std::make_shared<AstStruct>("__int32_array");
    int32ArrayStruct->addItem(Var(AstBuilder::buildPointerType(AstBuilder::buildInt32Type()), "ptr"), nullptr);
    int32ArrayStruct->addItem(Var(AstBuilder::buildInt32Type(), "size"), std::make_shared<AstInt>(0));
    tree->addStruct(int32ArrayStruct);
    
    // Int64
    auto int64ArrayStruct = std::make_shared<AstStruct>("__int64_array");
    int64ArrayStruct->addItem(Var(AstBuilder::buildPointerType(AstBuilder::buildInt64Type()), "ptr"), nullptr);
    int64ArrayStruct->addItem(Var(AstBuilder::buildInt32Type(), "size"), std::make_shared<AstInt>(0));
    tree->addStruct(int64ArrayStruct);
}

Parser::~Parser() {
}

bool Parser::parse() {
    int tk;
    do {
        tk = lex->get_next();
        bool code = true;
        
        switch (tk) {
            case t_import: code = build_import(); break;
        
            case t_extern:
            case t_func: {
                code = buildFunction(tk);
            } break;
            
            case t_const: code = buildConst(tree->block, true); break;
            case t_enum: code = buildEnum(); break;
            case t_struct: code = buildStruct(); break;
            case t_class: code = buildClass(); break;
            
            case t_eof: break;
            
            default: {
                syntax->addError(lex->line_number, "Invalid token in global scope.");
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

// Builds an import statement
bool Parser::build_import() {
    int token = lex->get_next();
    std::string path = "";

    while (token != t_semicolon) {
        switch (token) {
            case t_id: path += lex->value; break;
            case t_dot: path += "/"; break;
            
            default: {
                syntax->addError(lex->line_number, "Invalid token in import statement.");
                return false;
            }
        }
        
        token = lex->get_next();
    }

    // Load the include path
    // TODO: We need better path support
#ifdef DEV_LINK_MODE
    path = std::string(ORKA_HEADER_LOCATION) + "/" + path + ".oh";
#else
    path = "/usr/local/include/orka/" + path + ".oh";
#endif

    // Invoke another parser to load the path
    auto parser = std::make_unique<Parser>(path);
    parser->parse();
    auto tree2 = parser->tree;
    
    tree->block->mergeSymbols(tree2->block);
    for (auto const& stmt : tree2->block->block) {
        tree->block->addStatement(stmt);
    }
    
    for (auto const &s : tree2->structs) tree->addStruct(s);
    for (auto const &c : tree2->classes) tree->addClass(c);

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
            case t_class: code = buildClassDec(block); break;
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
            case t_repeat: code = buildRepeat(block); break;
            case t_for: code = buildFor(block); break;
            case t_forall: code = buildForAll(block); break;
            case t_break: code = buildLoopCtrl(block, true); break;
            case t_continue: code = buildLoopCtrl(block, false); break;
            
            // Annotated sub-block
            // TODO: Put in function
            case t_annot: {
                consume_token(t_id, "Expected block name.");
                auto name = lex->value;
                
                auto annot_block = std::make_shared<AstBlockStmt>(name);
                block->addStatement(annot_block);
                
                int t = lex->get_next();
                while (t != t_eof && t != t_is) {
                    if (t != t_id) {
                        syntax->addError(lex->line_number, "Expected name.");
                        return false;
                    }
                    
                    annot_block->clauses.push_back(lex->value);
                    t = lex->get_next();
                }
                
                if (t == t_eof) {
                    syntax->addError(lex->line_number, "Unexpected EOF in annotated block.");
                    return false;
                } else if (t != t_is) {
                    syntax->addError(lex->line_number, "Expected \"is\" in annotated block.");
                    return false;
                }
                
                annot_block->block->mergeSymbols(block);
                buildBlock(annot_block->block);
            } break;
            
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
        case t_float: dataType = AstBuilder::buildFloat32Type(); break;
        case t_double: dataType = AstBuilder::buildFloat64Type(); break;
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
            tk = lex->get_next();
            if (tk != t_rbracket) {
                syntax->addError(lex->line_number, "Invalid pointer type.");
                return nullptr;
            }
            
            //dataType = AstBuilder::buildPointerType(dataType);
            // TODO: Get this properly
            dataType = AstBuilder::buildStructType(getArrayType(dataType));
        } else {
            lex->unget(tk);
        }
    }
    
    return dataType;
}

std::string Parser::getArrayType(std::shared_ptr<AstDataType> dataType) {
    switch (dataType->type) {
        case V_AstType::Char:
        case V_AstType::Int8: return "__int8_array";
        case V_AstType::Int16: return "__int16_array";
        case V_AstType::Int64: return "__int64_array";
        case V_AstType::Float32: return "__f32_array";
        case V_AstType::Float64: return "__f64_array";
        
        default: {}
    }
    
    return "__int32_array";
}

//
// A helper function for getting and verifying a token
//
void Parser::consume_token(token t, std::string message) {
    int next = lex->get_next();
    if (t != next) {
        syntax->addError(lex->line_number, message);
    }
}

