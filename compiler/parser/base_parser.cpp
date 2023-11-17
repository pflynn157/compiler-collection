//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <ast/ast_builder.hpp>

#include "base_parser.hpp"

//
// Basic initialisation for the parser
//
BaseParser::BaseParser(std::string input) {
    this->input = input;
    this->tree = std::make_shared<AstTree>(input);
    this->syntax = std::make_shared<ErrorManager>();
}

// This is meant mainly for literals; it checks to make sure all the types in
// the expression agree in type. LLVM will have a problem if not
std::shared_ptr<AstExpression> BaseParser::checkExpression(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> varType) {
    if (!varType) return expr;

    switch (expr->type) {
        case V_AstType::I32L: {
            // Change to byte literals
            if (varType->type == V_AstType::Int8) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI8> byte = std::make_shared<AstI8>(i32->getValue());
                expr = byte;
                
            // Change to word literals
            } else if (varType->type == V_AstType::Int16) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI16> i16 = std::make_shared<AstI16>(i32->getValue());
                expr = i16;
                
            // Change to qword literals
            } else if (varType->type == V_AstType::Int64) {
                std::shared_ptr<AstI32> i32 = std::static_pointer_cast<AstI32>(expr);
                std::shared_ptr<AstI64> i64 = std::make_shared<AstI64>(i32->getValue());
                expr = i64;
            }
        } break;
            
        default: {}
    }
    
    return expr;
}

// Applies higher precedence for an operator
bool BaseParser::applyHigherPred(std::shared_ptr<ExprContext> ctx) {
    if (ctx->output.empty()) {
        syntax->addError(0, "Invalid expression: No RVAL");
        return false;
    }
    std::shared_ptr<AstExpression> rval = checkExpression(ctx->output.top(), ctx->varType);
    ctx->output.pop();
    
    if (ctx->output.empty()) {
        syntax->addError(0, "Invalid expression: No LVAL");
        return false;
    }
    std::shared_ptr<AstExpression> lval = checkExpression(ctx->output.top(), ctx->varType);
    ctx->output.pop();
    
    std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(ctx->opStack.top());
    ctx->opStack.pop();
    
    op->lval = lval;
    op->rval = rval;
    ctx->output.push(op);
    
    return true;
}

// Applies operator associativity
bool BaseParser::applyAssoc(std::shared_ptr<ExprContext> ctx) {
    V_AstType lastOp = V_AstType::None;
    while (ctx->opStack.size() > 0) {
        if (ctx->output.empty()) {
            syntax->addError(0, "Invalid expression: No RVAL");
            return false;
        }
        std::shared_ptr<AstExpression> rval = checkExpression(ctx->output.top(), ctx->varType);
        ctx->output.pop();
        
        if (ctx->output.empty()) {
            syntax->addError(0, "Invalid expression: No LVAL");
            return false;
        }
        std::shared_ptr<AstExpression> lval = checkExpression(ctx->output.top(), ctx->varType);
        ctx->output.pop();
        
        std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(ctx->opStack.top());
        ctx->opStack.pop();
        
        if (op->type == lastOp) {
            std::shared_ptr<AstBinaryOp> op2 = std::static_pointer_cast<AstBinaryOp>(rval);
            
            rval = op2->rval;
            op2->rval = op2->lval;
            op2->lval = lval;
            
            lval = op2;
        }
        
        op->lval = lval;
        op->rval = rval;
        ctx->output.push(op);
        
        lastOp = op->type;
    }
    
    return true;
}

//
// Runs post-processing on operators after they are built
//
void BaseParser::post_process_operator(std::shared_ptr<ExprContext> ctx,
                                       std::shared_ptr<AstBinaryOp> op, std::shared_ptr<AstUnaryOp> op1,
                                       bool is_unary) {
    if (ctx->opStack.size() > 0 && is_unary == false) {
        std::shared_ptr<AstOp> top = ctx->opStack.top();
            if (top->is_binary) {
            std::shared_ptr<AstBinaryOp> op2 = std::static_pointer_cast<AstBinaryOp>(top);
            if (op->precedence > op2->precedence) {
                if (!applyHigherPred(ctx)) return;
            }
        }
    }
    
    if (is_unary) ctx->opStack.push(op1);
    else ctx->opStack.push(op);
    ctx->lastWasOp = true;
}

