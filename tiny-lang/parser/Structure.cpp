//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
// Structure.cpp
// Handles parsing for structs
#include <map>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Parses and builds a structure
bool Parser::buildStruct() {
    consume_token(t_id, "Expected name for struct.");
    std::string name = lex->value;
    
    // Next token should be "is"
    consume_token(t_is, "Expected \"is\".");
    
    // Builds the struct items
    std::shared_ptr<AstStruct> str = std::make_shared<AstStruct>(name);
    int tk = lex->get_next();
    
    while (tk != t_end && tk != t_eof) {
        if (!buildStructMember(str, tk)) return false;
        tk = lex->get_next();
    }
    
    tree->addStruct(str);
    
    return true;
}

bool Parser::buildStructMember(std::shared_ptr<AstStruct> str, int tk) {
    std::string valName = lex->value;
    
    if (tk != t_id) {
        syntax->addError(lex->line_number, "Expected id value.");
        return false;
    }
        
    // Get the data type
    consume_token(t_colon, "Expected \':\' in structure member.");
    
    std::shared_ptr<AstDataType> dataType = buildDataType();
        
    // If its an array, build that. Otherwise, build the default value
    tk = lex->get_next();
        
    if (tk == t_assign) {
        std::shared_ptr<AstExpression> expr = buildExpression(nullptr, dataType, t_semicolon, true);
        if (!expr) return false;
                
        Var v;
        v.name = valName;
        v.type = dataType;
        str->addItem(v, expr);
    } else {
        syntax->addError(lex->line_number, "Expected default value.");
        return false;
    }
        
    return true;
}

bool Parser::buildStructDec(std::shared_ptr<AstBlock> block) {
    consume_token(t_id, "Expected structure name.");
    std::string name = lex->value;
    
    consume_token(t_colon, "Expected \':\'");
    
    consume_token(t_id, "Expected structure type.");
    std::string structName = lex->value;
    
    // Make sure the given structure exists
    std::shared_ptr<AstStruct> str = nullptr;
    
    for (auto s : tree->structs) {
        if (s->name == structName) {
            str = s;
            break;
        }    
    }
    
    if (str == nullptr) {
        syntax->addError(lex->line_number, "Unknown structure.");
        return false;
    }
    
    // Now build the declaration and push back
    block->addSymbol(name, AstBuilder::buildStructType(structName));
    std::shared_ptr<AstStructDec> dec = std::make_shared<AstStructDec>(name, structName);
    block->addStatement(dec);
    
    // Final syntax check
    int tk = lex->get_next();
    if (tk == t_semicolon) {
        return true;
    } else if (tk == t_assign) {
        dec->no_init = true;
        std::shared_ptr<AstExprStatement> empty = std::make_shared<AstExprStatement>();
        std::shared_ptr<AstExpression> arg = buildExpression(block, AstBuilder::buildStructType(structName), t_semicolon);
        if (!arg) return false;
        
        std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
        std::shared_ptr<AstAssignOp> assign = std::make_shared<AstAssignOp>(id, arg);
        
        empty->expression = assign;
        block->addStatement(empty);
        
        // TODO: The body should only be a function call expression or an ID
        // Do a syntax check
    }
    
    return true;
}

