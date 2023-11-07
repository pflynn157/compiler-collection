//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <memory>

#include "midend.hpp"

//
// Process binary operation for strings
//
std::shared_ptr<AstExpression> Midend::process_binary_op(std::shared_ptr<AstBinaryOp> expr, std::shared_ptr<AstBlock> block) {
    auto lval = expr->lval;
    auto rval = expr->rval;
    
    bool string_op = false;
    bool rval_str = false;
    
    if (lval->type == V_AstType::StringL || rval->type == V_AstType::StringL) {
        string_op = true;
        rval_str = true;
    } else if (lval->type == V_AstType::StringL && rval->type == V_AstType::CharL) {
        string_op = true;
    } else if (lval->type == V_AstType::ID && rval->type == V_AstType::CharL) {
        std::shared_ptr<AstID> lvalID = std::static_pointer_cast<AstID>(lval);
        if (block->getDataType(lvalID->value)->type == V_AstType::String) string_op = true;
    } else if (lval->type == V_AstType::ID && rval->type == V_AstType::ID) {
        std::shared_ptr<AstID> lvalID = std::static_pointer_cast<AstID>(lval);
        std::shared_ptr<AstID> rvalID = std::static_pointer_cast<AstID>(rval);
        
        if (block->getDataType(lvalID->value)->type == V_AstType::String) string_op = true;
        if (block->getDataType(rvalID->value)->type == V_AstType::String) {
            string_op = true;
            rval_str = true;
        } else if (block->getDataType(rvalID->value)->type == V_AstType::Char ||
                   block->getDataType(rvalID->value)->type == V_AstType::Int8) {
            string_op = true;          
        }
    }
    
    // Build the new comparison
    if (!string_op) {
        return nullptr;
    }
    
    auto args = std::make_shared<AstExprList>();
    args->add_expression(lval);
    args->add_expression(rval);

    if (expr->type == V_AstType::EQ || expr->type == V_AstType::NEQ) {
        auto fc = std::make_shared<AstFuncCallExpr>("stringcmp");
        fc->args = args;
        expr->lval = fc;
        
        if (expr->type == V_AstType::NEQ)
            expr->rval = std::make_shared<AstI32>(0);
        else
            expr->rval = std::make_shared<AstI32>(1);
    } else if (expr->type == V_AstType::Add) {
        if (rval_str) {
            auto fc = std::make_shared<AstFuncCallExpr>("strcat_str");
            fc->args = args;
            return fc;
        } else {
            auto fc = std::make_shared<AstFuncCallExpr>("strcat_char");
            fc->args = args;
            return fc;
        }
    }
    
    return nullptr;
}

