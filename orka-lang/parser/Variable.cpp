//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
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
    int tk = lex->get_next();
    std::vector<std::string> toDeclare;
    toDeclare.push_back(lex->value);
    
    if (tk != t_id) {
        syntax->addError(lex->line_number, "Expected variable name.");
        return false;
    }
    
    tk = lex->get_next();
    
    while (tk != t_colon) {
        if (tk == t_comma) {
            tk = lex->get_next();
            
            if (tk != t_id) {
                syntax->addError(lex->line_number, "Expected variable name.");
                return false;
            }
            
            toDeclare.push_back(lex->value);
        } else if (tk != t_colon) {
            syntax->addError(lex->line_number, "Invalt_id tk in variable declaration.");
            return false;
        }
        
        tk = lex->get_next();
    }
    
    std::shared_ptr<AstDataType> dataType = buildDataType(false);
    tk = lex->get_next();
    
    // We have an array
    if (tk == t_lbracket) {
        std::shared_ptr<AstDataType> baseType = dataType;
        std::string dataType_name = getArrayType(dataType);
        dataType = AstBuilder::buildStructType(dataType_name);
        std::shared_ptr<AstStructDec> dec = std::make_shared<AstStructDec>("", dataType_name);
        
        std::shared_ptr<AstExpression> size_arg = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
        if (!size_arg) {
            syntax->addError(lex->line_number, "Invalid size argument in array declaration");
            return false;
        }
        //empty->expression = arg; 
        
        tk = lex->get_next();
        if (tk != t_semicolon) {
            syntax->addError(lex->line_number, "Error: Expected \';\'.");
            return false;
        }
        
        for (std::string name : toDeclare) {
            std::shared_ptr<AstStructDec> dec = std::make_shared<AstStructDec>(name, dataType_name);
            dec->no_init = true;
            block->addStatement(dec);
            
            //
            // Malloc
            //
            std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
            
            // Get the size
            //std::shared_ptr<AstI32> size = std::make_shared<AstI32>(4);
            std::shared_ptr<AstInt> size;
            if (baseType->type == V_AstType::Int32) size = std::make_shared<AstInt>(4);
            else if (baseType->type == V_AstType::Int64) size = std::make_shared<AstInt>(8);
            else if (baseType->type == V_AstType::String) size = std::make_shared<AstInt>(8);
            else size = std::make_shared<AstInt>(1);
            
            std::shared_ptr<AstMulOp> mul_op = std::make_shared<AstMulOp>();
            mul_op->lval = size;
            mul_op->rval = size_arg;
            list->add_expression(mul_op);
            
            // Create the malloc call
            std::shared_ptr<AstFuncCallExpr> call_malloc = std::make_shared<AstFuncCallExpr>("gc_alloc");
            call_malloc->args = list;
            
            std::shared_ptr<AstStructAccess> lval_alloc = std::make_shared<AstStructAccess>(name, "ptr");
            std::shared_ptr<AstAssignOp> op1 = std::make_shared<AstAssignOp>();
            op1->lval = lval_alloc;
            op1->rval = call_malloc;
            
            std::shared_ptr<AstExprStatement> va_ptr = std::make_shared<AstExprStatement>();
            va_ptr->setDataType(dataType);
            va_ptr->expression = op1;
            block->addStatement(va_ptr);
            
            //
            // Set the size
            //
            // expression
            std::shared_ptr<AstStructAccess> lval_size = std::make_shared<AstStructAccess>(name, "size");
            std::shared_ptr<AstAssignOp> op = std::make_shared<AstAssignOp>();
            op->lval = lval_size;
            op->rval = size_arg;
            
            // The statement
            std::shared_ptr<AstExprStatement> va_size = std::make_shared<AstExprStatement>();
            va_size->setDataType(dataType);
            va_size->expression = op;
            block->addStatement(va_size);
            
            block->addSymbol(name, dataType);
        }
    
    // We're at the end of the declaration
    } else if (tk == t_semicolon) {
        syntax->addError(lex->line_number, "Expected init expression.");
        return false;
        
    // Otherwise, we have a regular variable
    } else {
        std::shared_ptr<AstExpression> arg = buildExpression(block, dataType, t_semicolon);
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
bool Parser::buildVariableAssign(std::shared_ptr<AstBlock> block, std::string value) {
    std::shared_ptr<AstDataType> dataType = block->getDataType(value);
    
    std::shared_ptr<AstExpression> expr = buildExpression(block, dataType, t_semicolon);
    if (!expr) return false;
    
    std::shared_ptr<AstExprStatement> stmt = std::make_shared<AstExprStatement>();
    stmt->setDataType(dataType);
    stmt->expression = expr;
    block->addStatement(stmt);
    
    return true;
}

// Builds a constant variable
bool Parser::buildConst(std::shared_ptr<AstBlock> block, bool isGlobal) {
    // Make sure we have a name for our constant
    consume_token(t_id, "Expected constant name.");
    std::string name = lex->value;
    
    // Syntax check
    consume_token(t_colon, "Expected \':\' in constant expression.");
    
    // Get the data type
    std::shared_ptr<AstDataType> dataType = buildDataType(false);
    
    // Final syntax check
    consume_token(t_assign, "Expected \'=\' after const assignment.");
    
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

