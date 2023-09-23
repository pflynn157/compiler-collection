//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Returns the function arguments
bool Parser::getFunctionArgs(std::shared_ptr<AstBlock> block, std::vector<Var> &args) {
    Token tk = scanner->getNext();
    if (tk.type == t_lparen) {
        tk = scanner->getNext();
        while (tk.type != t_eof && tk.type != t_rparen) {
            Token t1 = tk;
            std::string name = t1.id_val;
            Token t2 = scanner->getNext();
            Var v;
            
            if (t1.type != t_id) {
                syntax->addError(0, "Invalid function argument: Expected name.");
                return false;
            }
            
            if (t2.type != t_colon) {
                syntax->addError(0, "Invalid function argument: Expected \':\'.");
                return false;
            }
            
            v.type = buildDataType();
            v.name = name;
            
            tk = scanner->getNext();
            if (tk.type == t_comma) {
                tk = scanner->getNext();
            }
            
            args.push_back(v);
            block->addSymbol(v.name, v.type);
        }
    } else {
        scanner->rewind(tk);
    }
    
    return true;
}

// Builds a function
bool Parser::buildFunction(Token startToken, std::string className) {
    localConsts.clear();
    
    Token tk;
    bool isExtern = false;

    // Handle extern function
    if (startToken.type == t_extern) {
        isExtern = true;
    }

    // Make sure we have a function name
    tk = scanner->getNext();
    std::string funcName = tk.id_val;
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected function name.");
        return false;
    }
    
    // Get arguments
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    if (!getFunctionArgs(block, args)) return false;

    // Check to see if there's any return type
    //std::string retName = "";       // TODO: Do we need this?
    tk = scanner->getNext();
    std::shared_ptr<AstDataType> dataType;
    if (tk.type == t_arrow) {
        dataType = buildDataType();
        tk = scanner->getNext();
    }
    else dataType = AstBuilder::buildVoidType();
    
    // Do syntax error check
    if (tk.type == t_semicolon && !isExtern) {
        syntax->addError(0, "Expected \';\' for extern function.");
        return false;
    } else if (tk.type == t_is && isExtern) {
        syntax->addError(0, "Expected \'is\' keyword.");
        return false;
    }

    // Create the function object
    funcs.push_back(funcName);
    
    if (isExtern) {
        std::shared_ptr<AstExternFunction> ex = std::make_shared<AstExternFunction>(funcName);
        ex->args = args;
        ex->data_type = dataType;
        tree->addGlobalStatement(ex);
        return true;
    }
    
    std::shared_ptr<AstFunction> func = std::make_shared<AstFunction>(funcName);
    func->data_type = dataType;
    func->args = args;
    tree->addGlobalStatement(func);
    func->block->mergeSymbols(block);
    
    // Build the body
    int stopLayer = 0;
    if (!buildBlock(func->block)) return false;
    
    // Make sure we end with a return statement
    V_AstType lastType = func->block->getBlock().back()->type;
    if (lastType == V_AstType::Return) {
        std::shared_ptr<AstStatement> ret = func->block->getBlock().back();
        if (func->data_type->type == V_AstType::Void && ret->hasExpression()) {
            syntax->addError(0, "Cannot return from void function.");
            return false;
        } else if (!ret->hasExpression()) {
            syntax->addError(0, "Expected return value.");
            return false;
        }
    } else {
        if (func->data_type->type == V_AstType::Void) {
            func->addStatement(std::make_shared<AstReturnStmt>());
        } else {
            syntax->addError(0, "Expected return statement.");
            return false;
        }
    }
    
    return true;
}

// Builds a function call
bool Parser::buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken) {
    // Make sure the function exists
    if (!isFunc(idToken.id_val)) {
        syntax->addError(0, "Unknown function.");
        return false;
    }

    std::shared_ptr<AstFuncCallStmt> fc = std::make_shared<AstFuncCallStmt>(idToken.id_val);
    block->addStatement(fc);
    
    std::shared_ptr<AstExpression> args = buildExpression(block, nullptr, t_semicolon, false, true);
    if (!args) return false;
    fc->expression = args;
    
    // If we have a print call, we need to do some special processing
    if (idToken.id_val == "print") {
        std::string fmt = "";
        for (auto expr : std::static_pointer_cast<AstExprList>(args)->list) {
            switch (expr->type) {
                case V_AstType::CharL: fmt += "c"; break;
                case V_AstType::I8L:
                case V_AstType::I16L:
                case V_AstType::I32L:
                case V_AstType::I64L: fmt += "d"; break;
                case V_AstType::StringL: fmt += "s"; break;
                
                case V_AstType::ArrayAccess:
                case V_AstType::StructAccess:
                case V_AstType::ID: {
                    std::shared_ptr<AstDataType> dtype;
                    if (expr->type == V_AstType::ArrayAccess) {
                        std::string name = std::static_pointer_cast<AstArrayAccess>(expr)->value;
                        dtype = block->getDataType(name);
                        dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                    } else if (expr->type == V_AstType::StructAccess) {
                        auto sa = std::static_pointer_cast<AstStructAccess>(expr);
                        auto sa_type = std::static_pointer_cast<AstStructType>(block->getDataType(sa->var));
                        for (auto str : tree->structs) {
                            if (str->name == sa_type->name) {
                                for (auto v_item : str->items) {
                                    if (v_item.name == sa->member) {
                                        dtype = v_item.type;
                                    }
                                }
                            }
                        }
                        if (dtype->type == V_AstType::Ptr) {
                            dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                        }
                    } else {
                        std::string name = std::static_pointer_cast<AstID>(expr)->value;
                        dtype = block->getDataType(name);
                        if (dtype->type == V_AstType::Ptr) {
                            dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                        }
                    }
                    
                    switch (dtype->type) {
                        case V_AstType::Bool: fmt += "b"; break;
                        case V_AstType::Char: fmt += "c"; break;
                        case V_AstType::Int8:
                        case V_AstType::Int16:
                        case V_AstType::Int32:
                        case V_AstType::Int64: fmt += "d"; break;
                        case V_AstType::String: fmt += "s"; break;
                        default: {}
                    }
                } break;
                
                default: {}
            }
        }
        
        // Add the format
        auto args2 = std::static_pointer_cast<AstExprList>(args);
        std::shared_ptr<AstString> fmt_str = std::make_shared<AstString>(fmt);
        args2->list.insert(args2->list.begin(), fmt_str);
    }
    
    return true;
}

// Builds a return statement
bool Parser::buildReturn(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstReturnStmt> stmt = std::make_shared<AstReturnStmt>();
    block->addStatement(stmt);
    
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr);
    if (!arg) return false;
    stmt->expression = arg;
    
    return true;
}

