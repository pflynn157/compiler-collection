#include <ast/ast_builder.hpp>

#include "parser.hpp"

//
// Set everything up
//
Parser::Parser(std::string input) : BaseParser(input) {
    lex = std::make_unique<Lex>(input);
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
            case t_return: parse_return(block); break;
            
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
        case t_int: return AstBuilder::buildInt32Type();
        
        default:
            syntax->addError(lex->line_number, "Invalid data type.");
    }
    
    return nullptr;
}

