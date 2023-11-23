#include <iostream>

#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

#include "interpreter.hpp"

//
// Evaluates an expression
//
void AstInterpreter::run_expression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> type) {
    if (is_int_type(type)) run_iexpression(ctx, expr);
    else if (is_float_type(type)) run_fexpression(ctx, expr);
    else if (is_string_type(type)) run_sexpression(ctx, expr);
}

// Runs an integer-based expression
void AstInterpreter::run_iexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {
    switch (expr->type) {
        // Constants
        case V_AstType::IntL: {
            auto i = std::static_pointer_cast<AstInt>(expr);
            ctx->istack.push(i->value);
        } break;
        
        // Variables
        case V_AstType::ID: {
            auto id = std::static_pointer_cast<AstID>(expr);
            if (is_int_array(ctx, id->value)) {
                ctx->sstack.push(id->value);
            } else {
                ctx->istack.push(ctx->ivar_map[id->value]);
            }
        } break;
        
        case V_AstType::ArrayAccess: {
            auto acc = std::static_pointer_cast<AstArrayAccess>(expr);
            run_iexpression(ctx, acc->index);
            int idx = ctx->istack.top();
            ctx->istack.pop();
            ctx->istack.push(ctx->iarray_map[acc->value][idx]);
        } break;
        
        // Function call expression
        case V_AstType::FuncCallExpr: {
            auto fc = std::static_pointer_cast<AstFuncCallExpr>(expr);
            if (fc->name == "malloc" || fc->name == "gc_alloc") {
                auto args = std::static_pointer_cast<AstExprList>(fc->args);
                auto mul = std::static_pointer_cast<AstMulOp>(args->list[0]);
                run_iexpression(ctx, mul->rval);
                int length = ctx->istack.top();
                ctx->istack_array = std::vector<uint64_t>();
                for (int i = 0; i<length; i++) {
                    ctx->istack_array.push_back(0);
                }
                ctx->istack.pop();
            } else {
                auto func = function_map[fc->name];
                if (func && func->data_type->type == V_AstType::Ptr) {
                    auto array = call_function(ctx, fc->name, std::static_pointer_cast<AstExprList>(fc->args));
                    ctx->istack_array = *std::get_if<std::vector<uint64_t>>(&array);
                } else {
                    auto value = call_function(ctx, fc->name, std::static_pointer_cast<AstExprList>(fc->args));
                    ctx->istack.push(*std::get_if<uint64_t>(&value));
                }
            }
        } break;
        
        // Assign operator
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstAssignOp>(expr);
            run_iexpression(ctx, op->rval);
            
            switch (op->lval->type) {
                // Simple variables
                case V_AstType::ID: {
                    auto id = std::static_pointer_cast<AstID>(op->lval);
                    if (is_int_array(ctx, id->value)) {
                        ctx->iarray_map[id->value] = ctx->istack_array;
                        ctx->istack_array.clear();
                    } else {
                        ctx->ivar_map[id->value] = ctx->istack.top();
                        ctx->istack.pop();
                    }
                } break;
                
                // Array access
                case V_AstType::ArrayAccess: {
                    auto acc = std::static_pointer_cast<AstArrayAccess>(op->lval);
                    int value = ctx->istack.top();
                    ctx->istack.pop();
                    
                    run_iexpression(ctx, acc->index);
                    int idx = ctx->istack.top();
                    ctx->istack.pop();
                    
                    ctx->iarray_map[acc->value][idx] = value;
                } break;
                
                // Unknown lval
                default: {}
            }
        } break;
        
        // Operators
        case V_AstType::Add:
        case V_AstType::Sub:
        case V_AstType::Mul:
        case V_AstType::Div:
        case V_AstType::Mod:
        case V_AstType::And:
        case V_AstType::Or:
        case V_AstType::Xor:
        case V_AstType::Lsh:
        case V_AstType::Rsh:
        case V_AstType::EQ:
        case V_AstType::NEQ:
        case V_AstType::GT:
        case V_AstType::LT:
        case V_AstType::GTE:
        case V_AstType::LTE:
        {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            run_iexpression(ctx, op->lval);
            run_iexpression(ctx, op->rval);
            
            uint64_t rval = ctx->istack.top();
            ctx->istack.pop();
            uint64_t lval = ctx->istack.top();
            ctx->istack.pop();
            
            if (expr->type == V_AstType::Add) ctx->istack.push(lval + rval);
            else if (expr->type == V_AstType::Sub) ctx->istack.push(lval - rval);
            else if (expr->type == V_AstType::Mul) ctx->istack.push(lval * rval);
            else if (expr->type == V_AstType::Div) ctx->istack.push(lval / rval);
            else if (expr->type == V_AstType::Mod) ctx->istack.push(lval % rval);
            else if (expr->type == V_AstType::And) ctx->istack.push(lval & rval);
            else if (expr->type == V_AstType::Or)  ctx->istack.push(lval | rval);
            else if (expr->type == V_AstType::Xor) ctx->istack.push(lval ^ rval);
            else if (expr->type == V_AstType::Lsh) ctx->istack.push(lval << rval);
            else if (expr->type == V_AstType::Rsh) ctx->istack.push(lval >> rval);
            else if (expr->type == V_AstType::EQ)  ctx->istack.push((int)(lval == rval));
            else if (expr->type == V_AstType::NEQ) ctx->istack.push((int)(lval != rval));
            else if (expr->type == V_AstType::GT)  ctx->istack.push((int)(lval > rval));
            else if (expr->type == V_AstType::LT)  ctx->istack.push((int)(lval < rval));
            else if (expr->type == V_AstType::GTE) ctx->istack.push((int)(lval >= rval));
            else if (expr->type == V_AstType::LTE) ctx->istack.push((int)(lval <= rval));
        } break;
        
        default: {}
    }
}

// Runs a floating point expression
void AstInterpreter::run_fexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {

}

// Runs a string expression
void AstInterpreter::run_sexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {
    switch (expr->type) {
        // Constants
        case V_AstType::IntL: {
            auto i = std::static_pointer_cast<AstInt>(expr);
            ctx->sstack.push(std::to_string(i->value));
        } break;
        
        case V_AstType::CharL: {
            auto c = std::static_pointer_cast<AstChar>(expr);
            ctx->sstack.push(std::string(1, c->value));
        } break;
        
        case V_AstType::StringL: {
            auto s = std::static_pointer_cast<AstString>(expr);
            ctx->sstack.push(s->value);
        } break;
        
        // Variables
        case V_AstType::ID: {
            auto id = std::static_pointer_cast<AstID>(expr);
            if (is_string_array(ctx, id->value)) {
                ctx->sstack.push(id->value);
            } else {
                ctx->sstack.push(ctx->svar_map[id->value]);
            }
        } break;
        
        case V_AstType::ArrayAccess: {
            auto acc = std::static_pointer_cast<AstArrayAccess>(expr);
            run_iexpression(ctx, acc->index);
            int idx = ctx->istack.top();
            ctx->istack.pop();
            
            if (ctx->type_map[acc->value]->type == V_AstType::String) {
                char c = ctx->svar_map[acc->value][idx];
                ctx->sstack.push(std::string(1, c));
            } else {
                ctx->sstack.push(ctx->sarray_map[acc->value][idx]);
            }
        } break;
        
        // Function call expression
        case V_AstType::FuncCallExpr: {
            auto fc = std::static_pointer_cast<AstFuncCallExpr>(expr);
            if (fc->name == "malloc" || fc->name == "gc_alloc") {
                auto args = std::static_pointer_cast<AstExprList>(fc->args);
                auto mul = std::static_pointer_cast<AstMulOp>(args->list[0]);
                run_iexpression(ctx, mul->rval);
                int length = ctx->istack.top();
                ctx->sstack_array = std::vector<std::string>();
                for (int i = 0; i<length; i++) {
                    ctx->sstack_array.push_back("");
                }
                ctx->istack.pop();
            } else {
                auto func = function_map[fc->name];
                if (func && func->data_type->type == V_AstType::Ptr) {
                    auto array = call_function(ctx, fc->name, std::static_pointer_cast<AstExprList>(fc->args));
                    ctx->sstack_array = *std::get_if<std::vector<std::string>>(&array);
                } else {
                    auto value = call_function(ctx, fc->name, std::static_pointer_cast<AstExprList>(fc->args));
                    ctx->sstack.push(*std::get_if<std::string>(&value));
                }
            }
        } break;
        
        // Assign operator
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstAssignOp>(expr);
            run_sexpression(ctx, op->rval);
            
            switch (op->lval->type) {
                // Simple variables
                case V_AstType::ID: {
                    auto id = std::static_pointer_cast<AstID>(op->lval);
                    if (is_string_array(ctx, id->value)) {
                        ctx->sarray_map[id->value] = ctx->sstack_array;
                        ctx->sstack_array.clear();
                    } else {
                        ctx->svar_map[id->value] = ctx->sstack.top();
                        ctx->sstack.pop();
                    }
                } break;
                
                // Array access
                case V_AstType::ArrayAccess: {
                    auto acc = std::static_pointer_cast<AstArrayAccess>(op->lval);
                    std::string value = ctx->sstack.top();
                    ctx->sstack.pop();
                    
                    run_iexpression(ctx, acc->index);
                    int idx = ctx->istack.top();
                    ctx->istack.pop();
                    
                    ctx->sarray_map[acc->value][idx] = value;
                } break;
                
                // Unknown lval
                default: {}
            }
        } break;
        
        // Operators
        /*case V_AstType::Add:
        {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            run_iexpression(ctx, op->lval);
            run_iexpression(ctx, op->rval);
            
            uint64_t rval = ctx->istack.top();
            ctx->istack.pop();
            uint64_t lval = ctx->istack.top();
            ctx->istack.pop();
            
        } break;*/
        
        default: {}
    }
}

