//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <stack>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

// Builds a constant expression value
std::shared_ptr<AstExpression> Parser::buildConstExpr(token tk) {
    switch (tk) {
        case t_true: return std::make_shared<AstI32>(1);
        case t_false: return std::make_shared<AstI32>(0);
        case t_char_literal: return std::make_shared<AstChar>(lex_get_id(scanner)[0]);
        case t_int_literal: return std::make_shared<AstI32>(lex_get_int(scanner));
        case t_string_literal: return std::make_shared<AstString>(lex_get_id(scanner));
        
        default: {}
    }
    
    return nullptr;
}

bool Parser::buildOperator(token tk, std::shared_ptr<ExprContext> ctx) {
    switch (tk) {
        case t_assign:
        case t_plus: 
        case t_minus:
        case t_mul:
        case t_div:
        case t_mod:
        case t_dot:
        case t_and:
        case t_or:
        case t_xor:
        case t_eq:
        case t_neq:
        case t_gt:
        case t_lt:
        case t_gte:
        case t_lte:
        case t_lgand:
        case t_lgor: {
            std::shared_ptr<AstBinaryOp> op = std::make_shared<AstBinaryOp>();
            std::shared_ptr<AstUnaryOp> op1 = std::make_shared<AstUnaryOp>();
            bool useUnary = false;
            switch (tk) {
                case t_assign: op = std::make_shared<AstAssignOp>(); break;
                case t_plus: op = std::make_shared<AstAddOp>(); break;
                case t_mul: op = std::make_shared<AstMulOp>(); break;
                case t_div: op = std::make_shared<AstDivOp>(); break;
                case t_mod: op = std::make_shared<AstModOp>(); break;
                case t_and: op = std::make_shared<AstAndOp>(); break;
                case t_or: op = std::make_shared<AstOrOp>(); break;
                case t_xor: op = std::make_shared<AstXorOp>(); break;
                case t_eq: op = std::make_shared<AstEQOp>(); break;
                case t_neq: op = std::make_shared<AstNEQOp>(); break;
                case t_gt: op = std::make_shared<AstGTOp>(); break;
                case t_lt: op = std::make_shared<AstLTOp>(); break;
                case t_gte: op = std::make_shared<AstGTEOp>(); break;
                case t_lte: op = std::make_shared<AstLTEOp>(); break;
                case t_lgand: op = std::make_shared<AstLogicalAndOp>(); break;
                case t_lgor: op = std::make_shared<AstLogicalOrOp>(); break;
                case t_minus: {
                    if (ctx->lastWasOp) {
                        op1 = std::make_shared<AstNegOp>();
                        useUnary = true;
                    } else {
                        op = std::make_shared<AstSubOp>();
                    }
                } break;
            }
            
            if (ctx->opStack.size() > 0 && useUnary == false) {
                std::shared_ptr<AstOp> top = ctx->opStack.top();
                    if (top->is_binary) {
                    std::shared_ptr<AstBinaryOp> op2 = std::static_pointer_cast<AstBinaryOp>(top);
                    if (op->precedence > op2->precedence) {
                        if (!applyHigherPred(ctx)) return false;
                    }
                }
            }
            
            if (useUnary) ctx->opStack.push(op1);
            else ctx->opStack.push(op);
            ctx->lastWasOp = true;
        } break;
        
        default: {}
    }   
    
    return true;        
}

bool Parser::buildIDExpr(std::shared_ptr<AstBlock> block, token tk, std::shared_ptr<ExprContext> ctx) {
    ctx->lastWasOp = false;
    int currentLine = 0;

    std::string name = lex_get_id(scanner);
    if (ctx->varType && ctx->varType->type == V_AstType::Void) {
        ctx->varType = block->getDataType(name);
        if (ctx->varType && ctx->varType->type == V_AstType::Ptr)
            ctx->varType = std::static_pointer_cast<AstPointerType>(ctx->varType)->base_type;
    }
    
    tk = lex_get_next(scanner);
    if (tk == t_lbracket) {
        std::shared_ptr<AstExpression> index = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
        if (index == nullptr) {
            syntax->addError(0, "Invalid array reference.");
            return false;
        }
        
        std::shared_ptr<AstArrayAccess> acc = std::make_shared<AstArrayAccess>(name);
        acc->index = index;
        ctx->output.push(acc);
    } else if (tk == t_lparen) {
        if (currentLine != 0) {
            syntax->addWarning(0, "Function call on newline- possible logic error.");
        }
        
        if (!isFunc(name)) {
            syntax->addError(0, "Unknown function call.");
            return false;
        }
    
        std::shared_ptr<AstFuncCallExpr> fc = std::make_shared<AstFuncCallExpr>(name);
        std::shared_ptr<AstExpression> args = buildExpression(block, ctx->varType, t_rparen, false, true);
        fc->args = args;
        
        ctx->output.push(fc);
    } else if (tk == t_dot) {
        // TODO: Search for structures here

        token idToken = lex_get_next(scanner);
        if (idToken != t_id) {
            syntax->addError(0, "Expected identifier.");
            return false;
        }
        
        std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(name, lex_get_id(scanner));
        ctx->output.push(val);
    } else {
        int constVal = isConstant(name);
        if (constVal > 0) {
            if (constVal == 1) {
                std::shared_ptr<AstExpression> expr = globalConsts[name].second;
                ctx->output.push(expr);
            } else if (constVal == 2) {
                std::shared_ptr<AstExpression> expr = localConsts[name].second;
                ctx->output.push(expr);
            }
        } else {
            if (block->isVar(name)) {
                std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
                ctx->output.push(id);
            } else {
                syntax->addError(0, "Unknown variable: " + name);
                return false;
            }
        }
        
        lex_rewind(scanner, tk);
    }
    return true;
}

// Applies higher precedence for an operator
bool Parser::applyHigherPred(std::shared_ptr<ExprContext> ctx) {
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
bool Parser::applyAssoc(std::shared_ptr<ExprContext> ctx) {
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

// Our new expression builder
std::shared_ptr<AstExpression> Parser::buildExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                                                        token stopToken, bool isConst, bool buildList) {
    std::shared_ptr<ExprContext> ctx = std::make_shared<ExprContext>();
    if (currentType) ctx->varType = currentType;
    else ctx->varType = AstBuilder::buildVoidType();
    
    std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
    bool isList = buildList;
    
    token tk = lex_get_next(scanner);
    while (tk != t_eof && tk != stopToken) {
        switch (tk) {
            case t_true:
            case t_false:
            case t_char_literal:
            case t_int_literal:
            case t_string_literal: {
                ctx->lastWasOp = false;
                std::shared_ptr<AstExpression> expr = buildConstExpr(tk);
                ctx->output.push(expr);
            } break;
            
            case t_id: {
                if (!buildIDExpr(block, tk, ctx)) return nullptr;
            } break;
            
            case t_assign:
            case t_plus: 
            case t_minus:
            case t_mul:
            case t_div:
            case t_mod:
            case t_and:
            case t_or:
            case t_xor:
            case t_eq:
            case t_neq:
            case t_gt:
            case t_lt:
            case t_gte:
            case t_lte:
            case t_lgand:
            case t_lgor: {
                if (!buildOperator(tk, ctx)) {
                    return nullptr;
                }
            } break;
            
            case t_lparen: {
                std::shared_ptr<AstExpression> subExpr = buildExpression(block, ctx->varType, t_rparen, false, isList);
                if (!subExpr) {
                    return nullptr;
                }
                ctx->output.push(subExpr);
                ctx->lastWasOp = false;
            } break;
            
            // TODO: We need some syntax checking with this
            case t_rparen: break;
            
            case t_comma: {
                applyAssoc(ctx);
                std::shared_ptr<AstExpression> expr = checkExpression(ctx->output.top(), ctx->varType);
                list->add_expression(expr);
                while (ctx->output.size() > 0) ctx->output.pop();
                while (ctx->opStack.size() > 0) ctx->opStack.pop();
                isList = true;
            } break;
            
            default: {
                syntax->addError(0, "Invalid token in expression.");
                return nullptr;
            }
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
        
        tk = lex_get_next(scanner);
    }
    
    if (tk == t_eof) {
        syntax->addError(0, "Invalid expression-> missing \';\'.");
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

