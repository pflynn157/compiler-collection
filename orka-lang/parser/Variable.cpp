//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Builds a variable declaration
// A variable declaration is composed of an Alloca and optionally, an assignment
bool Parser::buildVariableDec(std::shared_ptr<AstBlock> block) {
    Token tk = scanner->getNext();
    std::vector<std::string> toDeclare;
    toDeclare.push_back(tk.id_val);
    
    if (tk.type != t_id) {
        syntax->addError(0, "Expected variable name.");
        return false;
    }
    
    tk = scanner->getNext();
    
    while (tk.type != t_colon) {
        if (tk.type == t_comma) {
            tk = scanner->getNext();
            
            if (tk.type != t_id) {
                syntax->addError(0, "Expected variable name.");
                return false;
            }
            
            toDeclare.push_back(tk.id_val);
        } else if (tk.type != t_colon) {
            syntax->addError(0, "Invalt_id tk in variable declaration.");
            return false;
        }
        
        tk = scanner->getNext();
    }
    
    std::shared_ptr<AstDataType> dataType = buildDataType(false);
    tk = scanner->getNext();
    
    // We have an array
    if (tk.type == t_lbracket) {
        dataType = AstBuilder::buildPointerType(dataType);
        std::shared_ptr<AstVarDec> empty = std::make_shared<AstVarDec>("", dataType);
        std::shared_ptr<AstExpression> arg = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
        if (!arg) return false;
        empty->expression = arg; 
        
        tk = scanner->getNext();
        if (tk.type != t_semicolon) {
            syntax->addError(0, "Error: Expected \';\'.");
            return false;
        }
        
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            block->addStatement(vd);
            vd->expression = empty->expression;
            
            // Create an assignment to a malloc call
            std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
            va->setDataType(dataType);
            block->addStatement(va);
            
            std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
            std::shared_ptr<AstFuncCallExpr> callMalloc = std::make_shared<AstFuncCallExpr>("malloc");
            std::shared_ptr<AstAssignOp> assign = std::make_shared<AstAssignOp>(id, callMalloc);
            
            va->expression = assign;
            
            // In order to get a proper malloc, we need to multiply the argument by
            // the size of the type. Get the arguments, and do that
            std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
            callMalloc->args = list;
            
            std::shared_ptr<AstI32> size;
            std::shared_ptr<AstDataType> baseType = std::static_pointer_cast<AstPointerType>(dataType)->base_type;
            if (baseType->type == V_AstType::Int32) size = std::make_shared<AstI32>(4);
            else if (baseType->type == V_AstType::Int64) size = std::make_shared<AstI32>(8);
            else if (baseType->type == V_AstType::String) size = std::make_shared<AstI32>(8);
            else size = std::make_shared<AstI32>(1);
            
            std::shared_ptr<AstMulOp> op = std::make_shared<AstMulOp>();
            op->lval = size;
            op->rval = vd->expression;
            list->add_expression(op);
            
            // Finally, set the size of the declaration
            vd->size = vd->expression;
            
            block->addSymbol(name, dataType);
        }
    
    // We're at the end of the declaration
    } else if (tk.type == t_semicolon) {
        syntax->addError(0, "Expected init expression.");
        return false;
        
    // Otherwise, we have a regular variable
    } else {
        std::shared_ptr<AstExpression> arg = buildExpression(block, dataType);
        if (!arg) return false;
    
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            block->addStatement(vd);
            
            std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
            std::shared_ptr<AstAssignOp> assign = std::make_shared<AstAssignOp>(id, arg);
            
            std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
            va->setDataType(dataType);
            va->expression = assign;
            block->addStatement(va);
            
            // Add the variable to the blocks symbol table
            block->addSymbol(name, dataType);
        }
    }
    
    return true;
}

// Builds a variable or an array assignment
bool Parser::buildVariableAssign(std::shared_ptr<AstBlock> block, Token t_idToken) {
    std::shared_ptr<AstDataType> dataType = block->getDataType(t_idToken.id_val);
    
    std::shared_ptr<AstExpression> expr = buildExpression(block, dataType);
    if (!expr) return false;
    
    std::shared_ptr<AstExprStatement> stmt = std::make_shared<AstExprStatement>();
    stmt->setDataType(dataType);
    stmt->expression = expr;
    block->addStatement(stmt);
    
    return true;
}

// Builds a constant variable
bool Parser::buildConst(std::shared_ptr<AstBlock> block, bool isGlobal) {
    Token tk = scanner->getNext();
    std::string name = tk.id_val;
    
    // Make sure we have a name for our constant
    if (tk.type != t_id) {
        syntax->addError(0, "Expected constant name.");
        return false;
    }
    
    // Syntax check
    tk = scanner->getNext();
    if (tk.type != t_colon) {
        syntax->addError(0, "Expected \':\' in constant expression.");
        return false;
    }
    
    // Get the data type
    std::shared_ptr<AstDataType> dataType = buildDataType(false);
    
    // Final syntax check
    tk = scanner->getNext();
    if (tk.type != t_assign) {
        syntax->addError(0, "Expected \'=\' after const assignment.");
        return false;
    }
    
    // Build the expression. We create a dummy statement for this
    std::shared_ptr<AstExpression> expr = buildExpression(nullptr, dataType, t_semicolon, true);
    if (!expr) return false;
    
    // Put it all together
    if (isGlobal) {
        block->globalConsts[name] = std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>(dataType, expr);
    } else {
        block->localConsts[name] = std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>(dataType, expr);
    }
    
    return true;
}

