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
        case t_true: return std::make_shared<AstInt>(1);
        case t_false: return std::make_shared<AstInt>(0);
        case t_char_literal: return std::make_shared<AstChar>((char)lex->i_value);
        case t_int_literal: {
            int value = lex->i_value;
            int tk_next = lex->get_next();
            if (tk_next == t_dot) {
                tk_next = lex->get_next();
                if (tk_next != t_int_literal) {
                    syntax->addError(lex->line_number, "Invalid integer or float literal.");
                    return nullptr;
                }
                
                int value2 = lex->i_value;
                std::string buffer = std::to_string(value) + "." + std::to_string(value2);
                double f_value = std::stod(buffer);
                return std::make_shared<AstFloat>(f_value);
            }
            lex->unget(tk_next);
            return std::make_shared<AstInt>(value);
        }
        case t_float_literal: return std::make_shared<AstFloat>(lex->f_value);
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
            syntax->addError(0, "Invalid array reference.");
            return false;
        }
        
        if (block->symbolTable[name]->type == V_AstType::String) {
            std::shared_ptr<AstArrayAccess> acc = std::make_shared<AstArrayAccess>(name);
            acc->index = index;
            ctx->output.push(acc);
        } else {
            std::shared_ptr<AstStructAccess> sa_acc = std::make_shared<AstStructAccess>(name, "ptr");
            sa_acc->access_expression = index;
            ctx->output.push(sa_acc);
        }
    } else if (tk == t_lparen) {
        if (currentLine != 0) {
            syntax->addWarning(0, "Function call on newline- possible logic error.");
        }
        
        if (!block->isFunc(name) && !java) {
            syntax->addError(0, "Unknown function call: " + name);
            return false;
        }
    
        std::shared_ptr<AstFuncCallExpr> fc = std::make_shared<AstFuncCallExpr>(name);
        std::shared_ptr<AstExpression> args = buildExpression(block, ctx->varType, t_rparen, false, true);
        fc->args = args;
        
        ctx->output.push(fc);
    } else if (tk == t_dot) {
        int idToken = lex->get_next();
        std::string id_value = lex->value;
        if (idToken != t_id) {
            syntax->addError(lex->line_number, "Expected identifier.");
            return false;
        }
        
        tk = lex->get_next();
        if (tk == t_lparen) {
            std::string className = classMap[name];
            std::string func_name = lex->value;
            if (!java) {
                func_name = className + "_" + lex->value;
            }
            
            auto fc = std::make_shared<AstFuncCallExpr>(func_name);
            auto id = std::make_shared<AstID>(name);
            fc->object_name = id->value;
            
            std::shared_ptr<AstExpression> args2 = buildExpression(block, ctx->varType, t_rparen, false, true);
            std::shared_ptr<AstExprList> args = std::static_pointer_cast<AstExprList>(args2);
            if (!java) args->list.insert(args->list.begin(), id);
            fc->args = args;
            
            ctx->output.push(fc);
        } else {
            lex->unget(tk);
            
            std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(name, id_value);
            ctx->output.push(val);
        }
        
    } else if (tk == t_scope) {
        if (enums.find(name) == enums.end()) {
            syntax->addError(lex->line_number, "Unknown enum.");
            return false;
        }
        
        tk = lex->get_next();
        if (tk != t_id) {
            syntax->addError(lex->line_number, "Expected identifier.");
            return false;
        }
        
        AstEnum dec = enums[name];
        std::shared_ptr<AstExpression> val = dec.values[lex->value];
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

bool Parser::is_constant(int tk) {
    switch (tk) {
        case t_true:
        case t_false:
        case t_char_literal:
        case t_int_literal:
        case t_float_literal:
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

bool Parser::build_other_token(int tk, bool is_const, std::shared_ptr<ExprContext> ctx) {
    switch (tk) {
        case t_sizeof: {
            ctx->lastWasOp = false;
            
            if (is_const) {
                syntax->addError(lex->line_number, "Invalid constant value.");
                return false;
            }
            
            std::string name = lex->value;
            
            int token1 = lex->get_next();
            int token2 = lex->get_next();
            std::string token2_value = lex->value;
            int token3 = lex->get_next();
            
            if (token1 != t_lparen || token2 != t_id || token3 != t_rparen) {
                syntax->addError(lex->line_number, "Invalid token in sizeof.");
                return false;
            }
            
            std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(token2_value, "size");
            ctx->output.push(val);
            
            return true;
        }
        
        default: {}
    }
    
    return false;
}

