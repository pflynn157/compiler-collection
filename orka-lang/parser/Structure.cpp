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

//
// Builds a class declaration
//
bool Parser::buildClass() {
    Token token = scanner->getNext();
    std::string name = token.id_val;
    std::string baseClass = "";
    
    if (token.type != t_id) {
        syntax->addError(scanner->getLine(), "Expected class name.");
        return false;
    }
    
    token = scanner->getNext();
    if (token.type == t_extends) {
        token = scanner->getNext();
        if (token.type != t_id) {
            syntax->addError(scanner->getLine(), "Expected extending class name.");
            return false;
        }
        
        baseClass = token.id_val;
        
        token = scanner->getNext();
        if (token.type != t_is) {
            syntax->addError(scanner->getLine(), "Expected \"is\".");
            return false;
        }
    } else if (token.type != t_is) {
        syntax->addError(scanner->getLine(), "Expected \"is\" or \"extends\".");
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
            syntax->addError(scanner->getLine(), "Unknown base class.");
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
            syntax->addError(scanner->getLine(), "Unknown base class.");
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
        token = scanner->getNext();
        bool code = true;
        
        switch (token.type) {
            case t_func: code = buildFunction(token, name); break;
            case t_var: {
                token = scanner->getNext();
                if (!buildStructMember(clazzStruct, token)) return false;
            } break; 
            
            case t_end: break;
            
            default: {
                syntax->addError(scanner->getLine(), "Invalid token in class.");
                token.print();
                code = false;
            }
        }
        
        if (!code) break;
    } while (token.type != t_end);
    
    currentClass = clazz;
    tree->addClass(clazz);
    
    return true;
}

// Builds a class declaration
// A class declaration is basically a structure declaration with a function call
//
bool Parser::buildClassDec(std::shared_ptr<AstBlock> block) {
    Token token = scanner->getNext();
    std::string name = token.id_val;
    
    if (token.type != t_id) {
        syntax->addError(scanner->getLine(), "Expected class name.");
        return false;
    }
    
    token = scanner->getNext();
    if (token.type != t_colon) {
        syntax->addError(scanner->getLine(), "Expected \":\"");
        return false;
    }
    
    token = scanner->getNext();
    std::string className = token.id_val;
    
    if (token.type != t_id) {
        syntax->addError(scanner->getLine(), "Expected class name.");
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
        syntax->addError(scanner->getLine(), "Unknown class.");
        return false;
    }
    
    // Build the structure declaration
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
    
    // Do the final syntax check
    token = scanner->getNext();
    if (token.type != t_semicolon) {
        syntax->addError(scanner->getLine(), "Expected terminator.");
        return false;
    }
    
    return true;
}

