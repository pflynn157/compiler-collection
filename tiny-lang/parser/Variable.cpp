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
    
    // We're at the end of the declaration
    if (tk == t_semicolon) {
        syntax->addError(lex->line_number, "Expected init expression.");
        return false;
    }
    
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
    
    return true;
}

//
// Builds an array declaration
//
bool Parser::build_array_dec(std::shared_ptr<AstBlock> block) {
    // Get the array name
    consume_token(t_id, "Expected array name.");
    std::string name = lex->value;
    
    // Get the colon
    consume_token(t_colon, "Expected \':\'.");
    
    // Build the data type
    std::shared_ptr<AstDataType> base_type = buildDataType(false);
    
    // Get the bracket
    consume_token(t_lbracket, "Expected opening \'[\'.");
    
    auto dataType = AstBuilder::buildPointerType(base_type);
    std::shared_ptr<AstExpression> arg = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
    if (!arg) return false;
    
    consume_token(t_semicolon, "Error: Expected \';\'.");
    
    std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
    block->addStatement(vd);
    vd->expression = arg;
    
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
    
    std::shared_ptr<AstInt> size;
    std::shared_ptr<AstDataType> baseType = std::static_pointer_cast<AstPointerType>(dataType)->base_type;
    if (baseType->type == V_AstType::Int32) size = std::make_shared<AstInt>(4);
    else if (baseType->type == V_AstType::Int64) size = std::make_shared<AstInt>(8);
    else if (baseType->type == V_AstType::String) size = std::make_shared<AstInt>(8);
    else size = std::make_shared<AstInt>(1);
    
    std::shared_ptr<AstMulOp> op = std::make_shared<AstMulOp>();
    op->lval = size;
    op->rval = vd->expression;
    list->add_expression(op);
    
    block->addSymbol(name, dataType);
    
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

