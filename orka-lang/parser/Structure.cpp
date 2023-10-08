//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
// Structure.cpp
// Handles parsing for structs
#include <map>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Parses and builds an enumeration
bool Parser::buildEnum() {
    Token token = scanner->getNext();
    std::string name = token.id_val;
    
    if (token.type != t_id) {
        syntax->addError(scanner->getLine(), "Expected enum name.");
        return false;
    }
    
    // See if we have a type for the enum. Default is int
    token = scanner->getNext();
    std::shared_ptr<AstDataType> dataType = AstBuilder::buildInt32Type();
    bool useDefault = false;
    
    switch (token.type) {
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
        
        case t_is: useDefault = true; break;
        
        default: {
            syntax->addError(scanner->getLine(), "Unknown token in enum declaration");
            return false;
        }
    }
    
    // Syntax check
    if (!useDefault) {
        token = scanner->getNext();
        if (token.type != t_is) {
            syntax->addError(scanner->getLine(), "Expected \"is\"");
            return false;
        }
    }
    
    // Loop and get all the values
    std::map<std::string, std::shared_ptr<AstExpression>> values;
    int index = 0;
    
    while (token.type != t_end && token.type != t_eof) {
        token = scanner->getNext();
        std::string valName = token.id_val;
        
        if (token.type != t_id) {
            syntax->addError(scanner->getLine(), "Expected enum value.");
            token.print();
            return false;
        }
        
        token = scanner->getNext();
        std::shared_ptr<AstExpression> value = nullptr;
        
        if (token.type == t_assign) {
        
        } else if (token.type != t_comma && token.type != t_end) {
            syntax->addError(scanner->getLine(), "Unknown token in enum.");
            token.print();
            return false;
        }
        
        if (value == nullptr) {
            value = checkExpression(std::make_shared<AstI32>(index), dataType);
            ++index;
        }
        
        values[valName] = value;
    }
    
    // Put it all together
    AstEnum theEnum;
    theEnum.name = name;
    theEnum.type = dataType;
    theEnum.values = values;
    enums[name] = theEnum;
    
    return true;
}

// Parses and builds a structure
bool Parser::buildStruct() {
    Token tk = scanner->getNext();
    std::string name = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected name for struct.");
        return false;
    }
    
    // Next token should be "is"
    tk = scanner->getNext();
    if (tk.type != t_is) {
        syntax->addError(0, "Expected \"is\".");
    }
    
    // Builds the struct items
    std::shared_ptr<AstStruct> str = std::make_shared<AstStruct>(name);
    tk = scanner->getNext();
    
    while (tk.type != t_end && tk.type != t_eof) {
        if (!buildStructMember(str, tk)) return false;
        tk = scanner->getNext();
    }
    
    tree->addStruct(str);
    
    return true;
}

bool Parser::buildStructMember(std::shared_ptr<AstStruct> str, Token tk) {
    std::string valName = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected id value.");
        //token.print();
        return false;
    }
        
    // Get the data type
    tk = scanner->getNext();
    if (tk.type != t_colon) {
        syntax->addError(0, "Expected \':\' in structure member.");
        //token.print();
        return false;
    }
    
    std::shared_ptr<AstDataType> dataType = buildDataType();
        
    // If its an array, build that. Otherwise, build the default value
    tk = scanner->getNext();
        
    if (tk.type == t_assign) {
        std::shared_ptr<AstExpression> expr = buildExpression(nullptr, dataType, t_semicolon, true);
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

bool Parser::buildStructDec(std::shared_ptr<AstBlock> block) {
    Token tk = scanner->getNext();
    std::string name = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected structure name.");
        return false;
    }
    
    tk = scanner->getNext();
    if (tk.type != t_colon) {
        syntax->addError(0, "Expected \':\'");
        return false;
    }
    
    tk = scanner->getNext();
    std::string structName = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected structure type.");
        return false;
    }
    
    // Make sure the given structure exists
    std::shared_ptr<AstStruct> str = nullptr;
    
    for (auto s : tree->structs) {
        if (s->name == structName) {
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
    std::shared_ptr<AstStructDec> dec = std::make_shared<AstStructDec>(name, structName);
    block->addStatement(dec);
    
    // Final syntax check
    tk = scanner->getNext();
    if (tk.type == t_semicolon) {
        return true;
    } else if (tk.type == t_assign) {
        dec->no_init = true;
        std::shared_ptr<AstExprStatement> empty = std::make_shared<AstExprStatement>();
        std::shared_ptr<AstExpression> arg = buildExpression(block, AstBuilder::buildStructType(structName));
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

