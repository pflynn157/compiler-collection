//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <memory>

#include "midend.hpp"

void Midend::process_function_call(std::shared_ptr<AstFuncCallStmt> call, std::shared_ptr<AstBlock> block) {
    if (call->name != "print") {
        return;
    }

    auto args = call->expression;
    std::string fmt = "";
    
    for (auto expr : std::static_pointer_cast<AstExprList>(args)->list) {
        switch (expr->type) {
            case V_AstType::CharL: fmt += "c"; break;
            case V_AstType::I8L:
            case V_AstType::I16L:
            case V_AstType::I32L:
            case V_AstType::I64L: fmt += "d"; break;
            case V_AstType::StringL: fmt += "s"; break;
            
            case V_AstType::ArrayAccess:
            case V_AstType::StructAccess:
            case V_AstType::ID: {
                std::shared_ptr<AstDataType> dtype;
                if (expr->type == V_AstType::ArrayAccess) {
                    std::string name = std::static_pointer_cast<AstArrayAccess>(expr)->value;
                    dtype = block->getDataType(name);
                    dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                } else if (expr->type == V_AstType::StructAccess) {
                    auto sa = std::static_pointer_cast<AstStructAccess>(expr);
                    auto sa_type = std::static_pointer_cast<AstStructType>(block->getDataType(sa->var));
                    for (auto str : tree->structs) {
                        if (str->name == sa_type->name) {
                            for (auto v_item : str->items) {
                                if (v_item.name == sa->member) {
                                    dtype = v_item.type;
                                }
                            }
                        }
                    }
                    if (dtype->type == V_AstType::Ptr) {
                        dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                    }
                } else {
                    std::string name = std::static_pointer_cast<AstID>(expr)->value;
                    dtype = block->getDataType(name);
                    if (dtype->type == V_AstType::Ptr) {
                        dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                    }
                }
                
                switch (dtype->type) {
                    case V_AstType::Bool: fmt += "b"; break;
                    case V_AstType::Char: fmt += "c"; break;
                    case V_AstType::Int8:
                    case V_AstType::Int16:
                    case V_AstType::Int32:
                    case V_AstType::Int64: fmt += "d"; break;
                    case V_AstType::String: fmt += "s"; break;
                    default: {}
                }
            } break;
            
            default: {}
        }
    }
    
    // Add the format
    auto args2 = std::static_pointer_cast<AstExprList>(args);
    std::shared_ptr<AstString> fmt_str = std::make_shared<AstString>(fmt);
    args2->list.insert(args2->list.begin(), fmt_str);
}

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

