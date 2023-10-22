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
std::shared_ptr<AstExpression> Parser::buildConstExpr(Token tk) {
    switch (tk.type) {
        case t_true: return std::make_shared<AstI32>(1);
        case t_false: return std::make_shared<AstI32>(0);
        case t_char_literal: return std::make_shared<AstChar>(tk.i8_val);
        case t_int_literal: return std::make_shared<AstI32>(tk.i32_val);
        case t_float_literal: return std::make_shared<AstFloat>(tk.float_val);
        case t_string_literal: return std::make_shared<AstString>(tk.id_val);
        
        default: {}
    }
    
    return nullptr;
}

bool Parser::buildOperator(Token tk, std::shared_ptr<ExprContext> ctx) {
    switch (tk.type) {
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
            switch (tk.type) {
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

bool Parser::buildIDExpr(std::shared_ptr<AstBlock> block, Token tk, std::shared_ptr<ExprContext> ctx) {
    ctx->lastWasOp = false;
    int currentLine = 0;

    std::string name = tk.id_val;
    if (ctx->varType && ctx->varType->type == V_AstType::Void) {
        ctx->varType = block->getDataType(name);
        if (ctx->varType && ctx->varType->type == V_AstType::Ptr)
            ctx->varType = std::static_pointer_cast<AstPointerType>(ctx->varType)->base_type;
    }
    
    tk = scanner->getNext();
    if (tk.type == t_lbracket) {
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
    } else if (tk.type == t_lparen) {
        if (currentLine != 0) {
            syntax->addWarning(0, "Function call on newline- possible logic error.");
        }
        
        if (!block->isFunc(name)) {
            syntax->addError(0, "Unknown function call: " + name);
            return false;
        }
    
        std::shared_ptr<AstFuncCallExpr> fc = std::make_shared<AstFuncCallExpr>(name);
        std::shared_ptr<AstExpression> args = buildExpression(block, ctx->varType, t_rparen, false, true);
        fc->args = args;
        
        ctx->output.push(fc);
    } else if (tk.type == t_dot) {
        Token idToken = scanner->getNext();
        if (idToken.type != t_id) {
            syntax->addError(scanner->getLine(), "Expected identifier.");
            return false;
        }
        
        tk = scanner->getNext();
        if (tk.type == t_lparen) {
            std::string className = classMap[name];
            className += "_" + idToken.id_val;
            
            auto fc = std::make_shared<AstFuncCallExpr>(className);
            auto id = std::make_shared<AstID>(name);
            
            std::shared_ptr<AstExpression> args2 = buildExpression(block, ctx->varType, t_rparen, false, true);
            std::shared_ptr<AstExprList> args = std::static_pointer_cast<AstExprList>(args2);
            args->list.insert(args->list.begin(), id);
            fc->args = args;
            
            ctx->output.push(fc);
        } else {
            scanner->rewind(tk);
            
            std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(name, idToken.id_val);
            ctx->output.push(val);
        }
        
    } else if (tk.type == t_scope) {
        if (enums.find(name) == enums.end()) {
            syntax->addError(scanner->getLine(), "Unknown enum.");
            return false;
        }
        
        tk = scanner->getNext();
        if (tk.type != t_id) {
            syntax->addError(scanner->getLine(), "Expected identifier.");
            return false;
        }
        
        AstEnum dec = enums[name];
        std::shared_ptr<AstExpression> val = dec.values[tk.id_val];
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
                syntax->addError(0, "Unknown variable: " + name);
                return false;
            }
        }
        
        scanner->rewind(tk);
    }
    return true;
}

// Our new expression builder
std::shared_ptr<AstExpression> Parser::buildExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                                                        TokenType stopToken, bool isConst, bool buildList) {
    std::shared_ptr<ExprContext> ctx = std::make_shared<ExprContext>();
    if (currentType) ctx->varType = currentType;
    else ctx->varType = AstBuilder::buildVoidType();
    
    std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
    bool isList = buildList;
    
    Token tk = scanner->getNext();
    while (tk.type != t_eof && tk.type != stopToken) {
        switch (tk.type) {
            case t_true:
            case t_false:
            case t_char_literal:
            case t_int_literal:
            case t_float_literal:
            case t_string_literal: {
                ctx->lastWasOp = false;
                std::shared_ptr<AstExpression> expr = buildConstExpr(tk);
                ctx->output.push(expr);
            } break;
            
            case t_id: {
                if (!buildIDExpr(block, tk, ctx)) return nullptr;
            } break;
            
            case t_sizeof: {
                ctx->lastWasOp = false;
                
                if (isConst) {
                    syntax->addError(scanner->getLine(), "Invalid constant value.");
                    return nullptr;
                }
                
                std::string name = tk.id_val;
                
                Token token1 = scanner->getNext();
                Token token2 = scanner->getNext();
                Token token3 = scanner->getNext();
                
                if (token1.type != t_lparen || token2.type != t_id || token3.type != t_rparen) {
                    syntax->addError(scanner->getLine(), "Invalid token in sizeof.");
                    tk.print();
                    return nullptr;
                }
                
                //auto id = std::make_shared<AstID>(token2.id_val);
                //auto size = std::make_shared<AstSizeof>(id);
                //ctx->output.push(size);
                
                std::shared_ptr<AstStructAccess> val = std::make_shared<AstStructAccess>(token2.id_val, "size");
                ctx->output.push(val);
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
                syntax->addError(0, "Invalid Token in expression.");
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
        
        tk = scanner->getNext();
    }
    
    if (tk.type == t_eof) {
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

