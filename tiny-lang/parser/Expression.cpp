//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <stack>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

// Builds a constant expression value
std::shared_ptr<AstExpression> Parser::build_constant(int tk) {
    switch (tk) {
        case t_true: return std::make_shared<AstI32>(1);
        case t_false: return std::make_shared<AstI32>(0);
        case t_char_literal: return std::make_shared<AstChar>((char)lex->i_value);
        case t_int_literal: return std::make_shared<AstI32>(lex->i_value);
        case t_string_literal: return std::make_shared<AstString>(lex->value);
        
        default: {}
    }
    
    return nullptr;
}

bool Parser::build_operator(int tk, std::shared_ptr<ExprContext> ctx) {
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
            
            post_process_operator(ctx, op, op1, useUnary);
        } break;
        
        default: {}
    }   
    
    return true;        
}

bool Parser::build_identifier(std::shared_ptr<AstBlock> block, int tk, std::shared_ptr<ExprContext> ctx) {
    ctx->lastWasOp = false;
    int currentLine = 0;

    std::string name = lex->value;
    if (ctx->varType && ctx->varType->type == V_AstType::Void) {
        ctx->varType = block->getDataType(name);
        if (ctx->varType && ctx->varType->type == V_AstType::Ptr)
            ctx->varType = std::static_pointer_cast<AstPointerType>(ctx->varType)->base_type;
    }
    
    tk = lex->get_next();
    if (tk == t_lbracket) {
        std::shared_ptr<AstExpression> index = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
        if (index == nullptr) {
            syntax->addError(lex->line_number, "Invalid array reference.");
            return false;
        }
        
        std::shared_ptr<AstArrayAccess> acc = std::make_shared<AstArrayAccess>(name);
        acc->index = index;
        ctx->output.push(acc);
    } else if (tk == t_lparen) {
        if (currentLine != 0) {
            syntax->addWarning(lex->line_number, "Function call on newline- possible logic error.");
        }
        
        if (!block->isFunc(name)) {
            syntax->addError(lex->line_number, "Unknown function call.");
            return false;
        }
    
        std::shared_ptr<AstFuncCallExpr> fc = std::make_shared<AstFuncCallExpr>(name);
        std::shared_ptr<AstExpression> args = buildExpression(block, ctx->varType, t_rparen, false, true);
        fc->args = args;
        
        ctx->output.push(fc);
    } else if (tk == t_dot) {
        // TODO: Search for structures here
        
        consume_token(t_id, "Expected identifier");
        std::string id_val = lex->value;
        
        std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(name, id_val);
        ctx->output.push(val);
    } else {
        int constVal = block->isConstant(name);
        if (constVal > 0) {
            if (constVal == 1) {
                std::shared_ptr<AstExpression> expr = block->globalConsts[name].second;
                ctx->output.push(expr);
            } else if (constVal == 2) {
                std::shared_ptr<AstExpression> expr = block->localConsts[name].second;
                ctx->output.push(expr);
            }
        } else {
            if (block->isVar(name)) {
                std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
                ctx->output.push(id);
            } else {
                syntax->addError(lex->line_number, "Unknown variable: " + name);
                return false;
            }
        }
        
        lex->unget(tk);
    }
    return true;
}

// Potential helper function
// TODO: Overload all of the "is" functions
bool Parser::is_constant(int tk) {
    switch (tk) {
        case t_true:
        case t_false:
        case t_char_literal:
        case t_int_literal:
        case t_string_literal: return true;
        
        default: {}
    }
    return false;
}

bool Parser::is_id(int tk) {
    if (tk == t_id) return true;
    return false;
}

bool Parser::is_operator(int tk) {
    switch (tk) {
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
        case t_lgor: return true;
        
        default: {}
    }
    return false;
}

bool Parser::is_sub_expr_start(int tk) {
    if (tk == t_lparen) return true;
    return false;
}

bool Parser::is_sub_expr_end(int tk) {
    if (tk == t_rparen) return true;
    return false;
}

bool Parser::is_list_delim(int tk) {
    if (tk == t_comma) return true;
    return false;
}

int Parser::get_sub_expr_end() {
    return t_rparen;
}

