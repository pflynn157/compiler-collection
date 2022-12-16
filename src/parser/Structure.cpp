//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
// Structure.cpp
// Handles parsing for structs
#include <map>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

// Parses and builds a structure
bool Parser::buildStruct() {
    token tk = lex_get_next(scanner);
    std::string name = lex_get_id(scanner);
    
    if (tk != t_id) {
        syntax->addError(0, "Expected name for struct.");
        return false;
    }
    
    // Next token should be "is"
    tk = lex_get_next(scanner);
    if (tk != t_is) {
        syntax->addError(0, "Expected \"is\".");
    }
    
    // Builds the struct items
    AstStruct *str = new AstStruct(name);
    tk = lex_get_next(scanner);
    
    while (tk != t_end && tk != t_eof) {
        if (!buildStructMember(str, tk)) return false;
        tk = lex_get_next(scanner);
    }
    
    tree->addStruct(str);
    
    return true;
}

bool Parser::buildStructMember(AstStruct *str, token tk) {
    std::string valName = lex_get_id(scanner);
    
    if (tk != t_id) {
        syntax->addError(0, "Expected id value.");
        //token.print();
        return false;
    }
        
    // Get the data type
    tk = lex_get_next(scanner);
    if (tk != t_colon) {
        syntax->addError(0, "Expected \':\' in structure member.");
        //token.print();
        return false;
    }
    
    AstDataType *dataType = buildDataType();
        
    // If its an array, build that. Otherwise, build the default value
    tk = lex_get_next(scanner);
        
    if (tk == t_assign) {
        AstExpression *expr = nullptr;
        expr = buildExpression(nullptr, dataType, t_semicolon, true);
        if (!expr) return false;
                
        Var v;
        v.name = valName;
        v.type = dataType;
        str->addItem(v, expr);
    } else {
        syntax->addError(0, "Expected default value.");
        //token.print();
        return false;
    }
        
    return true;
}

bool Parser::buildStructDec(AstBlock *block) {
    token tk = lex_get_next(scanner);
    std::string name = lex_get_id(scanner);
    
    if (tk != t_id) {
        syntax->addError(0, "Expected structure name.");
        return false;
    }
    
    tk = lex_get_next(scanner);
    if (tk != t_colon) {
        syntax->addError(0, "Expected \':\'");
        return false;
    }
    
    tk = lex_get_next(scanner);
    std::string structName = lex_get_id(scanner);
    
    if (tk != t_id) {
        syntax->addError(0, "Expected structure type.");
        return false;
    }
    
    // Make sure the given structure exists
    AstStruct *str = nullptr;
    
    for (auto s : tree->getStructs()) {
        if (s->getName() == structName) {
            str = s;
            break;
        }    
    }
    
    if (str == nullptr) {
        syntax->addError(0, "Unknown structure.");
        return false;
    }
    
    // Now build the declaration and push back
    block->addSymbol(name, AstBuilder::buildStructType(structName));
    AstStructDec *dec = new AstStructDec(name, structName);
    block->addStatement(dec);
    
    // Final syntax check
    tk = lex_get_next(scanner);
    if (tk == t_semicolon) {
        return true;
    } else if (tk == t_assign) {
        dec->setNoInit(true);
        AstExprStatement *empty = new AstExprStatement;
        AstExpression *arg = buildExpression(block, AstBuilder::buildStructType(structName));
        if (!arg) return false;
        
        AstID *id = new AstID(name);
        AstAssignOp *assign = new AstAssignOp(id, arg);
        
        empty->setExpression(assign);
        block->addStatement(empty);
        
        // TODO: The body should only be a function call expression or an ID
        // Do a syntax check
    }
    
    return true;
}

