#include <ast/ast_builder.hpp>

#include "parser.hpp"

//
// Set everything up
//
Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
    
    // Add built-in functions
    auto fc1 = std::make_shared<AstExternFunction>("printf");
    fc1->data_type = AstBuilder::buildVoidType();
    fc1->varargs = true;
    fc1->addArgument(Var(AstBuilder::buildStringType(), "fmt"));
    tree->block->addStatement(fc1);
    tree->block->funcs.push_back("print");
    
    // void __kmpc_fork_call(int *global_id, int *bound_id, int *func)
    tree->block->funcs.push_back("__kmpc_fork_call");
    auto omp_fc1 = std::make_shared<AstExternFunction>("__kmpc_fork_call");
    omp_fc1->addArgument(Var(AstBuilder::buildInt32PointerType(), "global_id"));
    omp_fc1->addArgument(Var(AstBuilder::buildInt32PointerType(), "bound_id"));
    omp_fc1->varargs = true;
    omp_fc1->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(omp_fc1);
    
    // void __kmpc_for_static_init_4(0, *global_id, 34, &last, &lower, &upper, &stride, 1, 1);
    tree->block->funcs.push_back("__kmpc_for_static_init_4");
    auto omp_fc2 = std::make_shared<AstExternFunction>("__kmpc_for_static_init_4");
    omp_fc2->addArgument(Var(AstBuilder::buildInt32PointerType(), "global_id"));
    omp_fc2->addArgument(Var(AstBuilder::buildInt32PointerType(), "bound_id"));
    omp_fc2->addArgument(Var(AstBuilder::buildInt32Type(), "schedule"));
    omp_fc2->varargs = true;
    omp_fc2->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(omp_fc2);
    
    // void __kmpc_for_static_fini(0, *global_id);
    tree->block->funcs.push_back("__kmpc_for_static_fini");
    auto omp_fc3 = std::make_shared<AstExternFunction>("__kmpc_for_static_fini");
    omp_fc3->addArgument(Var(AstBuilder::buildInt32PointerType(), "global_id"));
    omp_fc3->addArgument(Var(AstBuilder::buildInt32PointerType(), "bound_id"));
    omp_fc3->varargs = true;
    omp_fc3->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(omp_fc3);
}

//
// The main parse loop
//
bool Parser::parse() {
    token t = lex->get_next();
    while (t != t_eof) {
        switch (t) {
            // Modules- for the name space
            case t_module: {
                consume_token(t_id, "Expected module name.");
                module_name = lex->value;
                
                consume_token(t_period, "Expected \'.\'");
            } break;
            
            // Functions
            case t_func: parse_function(); break;
            
            // Syntax error
            default: {
                syntax->addError(lex->line_number, "Invalid token in global scope.");
            }
        }
        
        t = lex->get_next();
    }

    //
    // Print any warnings and errors
    //
    syntax->printWarnings();
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    return true;
}

//
// The loop to parse any block of statements
//
void Parser::parse_block(std::shared_ptr<AstBlock> block) {
    token t = lex->get_next();
    while (t != t_eof && t != t_end) {
        switch (t) {
            // Statements in separate functions
            case t_scalar: parse_scalar(block); break;
            case t_return: parse_return(block); break;
            case t_iter: parse_iter(block); break;
            
            // Statements beginning with an identifier
            case t_id: {
                std::string name = lex->value;
                token next = lex->get_next();
                lex->unget(next);
                
                // TODO: Check assign
                parse_function_call(block, name);
            } break;
            
            // Annotated sub-block
            case t_annot: {
                consume_token(t_id, "Expected block name.");
                auto name = lex->value;
                
                auto annot_block = std::make_shared<AstBlockStmt>(name);
                block->addStatement(annot_block);
                
                t = lex->get_next();
                while (t != t_eof && t != t_is) {
                    if (t != t_id) {
                        syntax->addError(lex->line_number, "Expected name.");
                        return;
                    }
                    
                    annot_block->clauses.push_back(lex->value);
                    t = lex->get_next();
                }
                
                if (t == t_eof) {
                    syntax->addError(lex->line_number, "Unexpected EOF in annotated block.");
                    return;
                } else if (t != t_is) {
                    syntax->addError(lex->line_number, "Expected \"is\" in annotated block.");
                    return;
                }
                
                annot_block->block->mergeSymbols(block);
                parse_block(annot_block->block);
            } break;
            
            // Syntax error
            default: {
                syntax->addError(lex->line_number, "Unexpected token in block.");
            }
        }
        
        t = lex->get_next();
    }
    
    if (t == t_eof) {
        syntax->addError(lex->line_number, "Unexpected end of file.");
    }
}

//
// A helper function for getting and verifying a token
//
void Parser::consume_token(token t, std::string message) {
    token next = lex->get_next();
    if (t != next) {
        syntax->addError(lex->line_number, message);
    }
}

//
// Another helper function for generating a base name from a module name
//
std::string Parser::generate_name(std::string base) {
    std::string name = "";
    if (module_name != "") {
        name = module_name + "_" + base;
    } else {
        name = base;
    }
    return name;
}

//
// A helper function for parsing data types
//
std::shared_ptr<AstDataType> Parser::get_data_type() {
    token t = lex->get_next();
    switch (t) {
        case t_void: return AstBuilder::buildVoidType();
        case t_int: return AstBuilder::buildInt32Type();
        
        default:
            syntax->addError(lex->line_number, "Invalid data type.");
    }
    
    return nullptr;
}

