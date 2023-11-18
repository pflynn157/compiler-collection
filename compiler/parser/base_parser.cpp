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
        case V_AstType::IntL: {
            // Change to byte literals
            if (varType->type == V_AstType::Int8) {
                std::shared_ptr<AstInt> i32 = std::static_pointer_cast<AstInt>(expr);
                i32->size = 8;
                expr = i32;
                
            // Change to word literals
            } else if (varType->type == V_AstType::Int16) {
                std::shared_ptr<AstInt> i32 = std::static_pointer_cast<AstInt>(expr);
                i32->size = 16;
                expr = i32;
                
            // Change to qword literals
            } else if (varType->type == V_AstType::Int64) {
                std::shared_ptr<AstInt> i32 = std::static_pointer_cast<AstInt>(expr);
                i32->size = 64;
                expr = i32;
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

// Our new expression builder
// This should be universal
std::shared_ptr<AstExpression> BaseParser::buildExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                                                        int stopToken, bool isConst, bool buildList) {
    std::shared_ptr<ExprContext> ctx = std::make_shared<ExprContext>();
    if (currentType) ctx->varType = currentType;
    else ctx->varType = AstBuilder::buildVoidType();
    
    std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
    bool isList = buildList;
    
    int tk = lex->get_next();
    while (tk != 0 && tk != stopToken) {
        if  (is_constant(tk)) {
            ctx->lastWasOp = false;
            std::shared_ptr<AstExpression> expr = build_constant(tk);
            ctx->output.push(expr);
        } else if (is_id(tk)) {
            if (!build_identifier(block, tk, ctx)) return nullptr;
        } else if (is_operator(tk)) {
            if (!build_operator(tk, ctx)) {
                return nullptr;
            }
        } else if (is_sub_expr_start(tk)) {
            std::shared_ptr<AstExpression> subExpr = buildExpression(block, ctx->varType, get_sub_expr_end(), false, isList);
            if (!subExpr) {
                return nullptr;
            }
            ctx->output.push(subExpr);
            ctx->lastWasOp = false;
        } else if (is_sub_expr_end(tk)) {
            // Do nothing
        } else if (is_list_delim(tk)) {
            applyAssoc(ctx);
            std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
            list->add_expression(expr);
            while (ctx->output.size() > 0) ctx->output.pop();
            while (ctx->opStack.size() > 0) ctx->opStack.pop();
            isList = true;
        } else {
            syntax->addError(lex->line_number, "Invalid Token in expression.");
            return nullptr;
        }
        
        if (!ctx->lastWasOp && ctx->opStack.size() > 0) {
            if (ctx->opStack.top()->type == V_AstType::Neg) {
                std::shared_ptr<AstExpression> val = checkExpression(ctx->output.top(), ctx->varType);
                ctx->output.pop();
                
                std::shared_ptr<AstNegOp> op = std::static_pointer_cast<AstNegOp>(ctx->opStack.top());
                ctx->opStack.pop();
                op->value = val;
                ctx->output.push(op);
            }
        }
        
        tk = lex->get_next();
    }
    
    if (tk == 0) {
        syntax->addError(lex->line_number, "Invalid expression-> missing \';\'.");
        return nullptr;
    }
    
    // Build the expression
    applyAssoc(ctx);
    
    
    if (ctx->output.size() == 0) {
        return list;
    }
    
    // Type check the top
    std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
    
    if (isList) {
        list->add_expression(expr);
        return list;
    }
    return expr;
}

