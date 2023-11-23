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

// Parses and builds an enumeration
bool Parser::buildEnum() {
    int token = lex->get_next();
    std::string name = lex->value;
    
    if (token != t_id) {
        syntax->addError(lex->line_number, "Expected enum name.");
        return false;
    }
    
    // See if we have a type for the enum. Default is int
    token = lex->get_next();
    std::shared_ptr<AstDataType> dataType = AstBuilder::buildInt32Type();
    bool useDefault = false;
    
    switch (token) {
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
            syntax->addError(lex->line_number, "Unknown token in enum declaration");
            return false;
        }
    }
    
    // Syntax check
    if (!useDefault) {
        token = lex->get_next();
        if (token != t_is) {
            syntax->addError(lex->line_number, "Expected \"is\"");
            return false;
        }
    }
    
    // Loop and get all the values
    std::map<std::string, std::shared_ptr<AstExpression>> values;
    int index = 0;
    
    while (token != t_end && token != t_eof) {
        token = lex->get_next();
        std::string valName = lex->value;
        
        if (token != t_id) {
            syntax->addError(lex->line_number, "Expected enum value.");
            return false;
        }
        
        token = lex->get_next();
        std::shared_ptr<AstExpression> value = nullptr;
        
        if (token == t_assign) {
        
        } else if (token != t_comma && token != t_end) {
            syntax->addError(lex->line_number, "Unknown token in enum.");
            return false;
        }
        
        if (value == nullptr) {
            value = checkExpression(std::make_shared<AstInt>(index), dataType);
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
    int tk = lex->get_next();
    std::string name = lex->value;
    
    if (tk != t_id) {
        syntax->addError(lex->line_number, "Expected name for struct.");
        return false;
    }
    
    // Next token should be "is"
    tk = lex->get_next();
    if (tk != t_is) {
        syntax->addError(lex->line_number, "Expected \"is\".");
    }
    
    // Builds the struct items
    std::shared_ptr<AstStruct> str = std::make_shared<AstStruct>(name);
    tk = lex->get_next();
    
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
    tk = lex->get_next();
    if (tk != t_colon) {
        syntax->addError(lex->line_number, "Expected \':\' in structure member.");
        return false;
    }
    
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
    int tk = lex->get_next();
    std::string name = lex->value;
    
    if (tk != t_id) {
        syntax->addError(lex->line_number, "Expected structure name.");
        return false;
    }
    
    tk = lex->get_next();
    if (tk != t_colon) {
        syntax->addError(lex->line_number, "Expected \':\'");
        return false;
    }
    
    tk = lex->get_next();
    std::string structName = lex->value;
    
    if (tk != t_id) {
        syntax->addError(lex->line_number, "Expected structure type.");
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
        syntax->addError(lex->line_number, "Unknown structure.");
        return false;
    }
    
    // Now build the declaration and push back
    block->addSymbol(name, AstBuilder::buildStructType(structName));
    std::shared_ptr<AstStructDec> dec = std::make_shared<AstStructDec>(name, structName);
    block->addStatement(dec);
    
    // Final syntax check
    tk = lex->get_next();
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

//
// Builds a class declaration
//
bool Parser::buildClass() {
    int token = lex->get_next();
    std::string name = lex->value;
    std::string baseClass = "";
    
    if (token != t_id) {
        syntax->addError(lex->line_number, "Expected class name.");
        return false;
    }
    
    token = lex->get_next();
    if (token == t_extends) {
        token = lex->get_next();
        if (token != t_id) {
            syntax->addError(lex->line_number, "Expected extending class name.");
            return false;
        }
        
        baseClass = lex->value;
        
        token = lex->get_next();
        if (token != t_is) {
            syntax->addError(lex->line_number, "Expected \"is\".");
            return false;
        }
    } else if (token != t_is) {
        syntax->addError(lex->line_number, "Expected \"is\" or \"extends\".");
        return false;
    }
    
    auto clazzStruct = std::make_shared<AstStruct>(name);
    tree->addStruct(clazzStruct);
    
    auto clazz = std::make_shared<AstClass>(name);
    currentClass = clazz;
    
    if (baseClass != "") {
        // First, build the inherited structure
        std::shared_ptr<AstStruct> baseStruct = nullptr;
        for (auto s : tree->structs) {
            if (s->name == baseClass) {
                baseStruct = s;
                break;
            }
        }
        
        if (baseStruct == nullptr) {
            syntax->addError(lex->line_number, "Unknown base class.");
            return false;
        }
        
        for (auto var : baseStruct->items) {
            clazzStruct->addItem(var, baseStruct->default_expressions[var.name]);
        }
        
        // Next, build the inherited class
        std::shared_ptr<AstClass> baseAstClass = nullptr;
        for (auto c : tree->classes) {
            if (c->name == baseClass) {
                baseAstClass = c;
                break;
            }
        }
        
        if (baseAstClass == nullptr) {
            syntax->addError(lex->line_number, "Unknown base class.");
            return false;
        }
        
        for (auto func : baseAstClass->functions) {
            if (func->name == baseClass) continue;
            clazz->addFunction(func);
            
            // Add it to the function scope in the AST tree
            std::string newName = name + "_" + func->name;
            
            // Copy it
            auto func2 = std::make_shared<AstFunction>(newName);
            func2->data_type = func->data_type;
            func2->args = func->args;
            tree->addGlobalStatement(func2);
            
            std::shared_ptr<AstBlock> block2 = func->block;
            func2->block->addStatements(block2->getBlock());
        }
    }
    
    do {
        token = lex->get_next();
        bool code = true;
        
        switch (token) {
            case t_func: code = buildFunction(token, name); break;
            case t_var: {
                token = lex->get_next();
                if (!buildStructMember(clazzStruct, token)) return false;
            } break; 
            
            case t_end: break;
            
            default: {
                syntax->addError(lex->line_number, "Invalid token in class.");
                code = false;
            }
        }
        
        if (!code) break;
    } while (token != t_end);
    
    currentClass = clazz;
    tree->addClass(clazz);
    
    return true;
}

// Builds a class declaration
// A class declaration is basically a structure declaration with a function call
//
bool Parser::buildClassDec(std::shared_ptr<AstBlock> block) {
    int token = lex->get_next();
    std::string name = lex->value;
    
    if (token != t_id) {
        syntax->addError(lex->line_number, "Expected class name.");
        return false;
    }
    
    token = lex->get_next();
    if (token != t_colon) {
        syntax->addError(lex->line_number, "Expected \":\"");
        return false;
    }
    
    token = lex->get_next();
    std::string className = lex->value;
    
    if (token != t_id) {
        syntax->addError(lex->line_number, "Expected class name.");
        return false;
    }
    
    // Make sure the structure exists, and names a class
    // TODO: Do the class check
    std::shared_ptr<AstStruct> str = nullptr;
    
    for (auto s : tree->structs) {
        if (s->name == className) {
            str = s;
            break;
        }
    }
    
    if (str == nullptr) {
        syntax->addError(lex->line_number, "Unknown class.");
        return false;
    }
    
    // Build the structure declaration
    if (java) {
        auto data_type = AstBuilder::buildObjectType(className);
        auto dec = std::make_shared<AstVarDec>(name, data_type);
        dec->class_name = className;
        block->addStatement(dec);
        
        classMap[name] = className;
        
    } else {
        auto dec = std::make_shared<AstStructDec>(name, className);
        block->addStatement(dec);
        
        classMap[name] = className;
        
        // Call the constructor
        auto classRef = std::make_shared<AstID>(name);
        auto args = std::make_shared<AstExprList>();
        args->add_expression(classRef);
        
        std::string constructor = className + "_" + className;
        auto fc = std::make_shared<AstFuncCallStmt>(constructor);
        block->addStatement(fc);
        fc->expression = args;
    }
    
    // Do the final syntax check
    consume_token(t_semicolon, "Expected terminator.");
    
    return true;
}

