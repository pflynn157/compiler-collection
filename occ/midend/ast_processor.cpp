#include <ast/ast.hpp>

#include "ast_processor.hpp"

//
// Various initialization functions
//
AstProcessor::AstProcessor(std::shared_ptr<AstTree> tree) {
    this->tree = tree;
}

void AstProcessor::run() {
    for (auto gs : tree->global_statements) {
        proc_global_statement(gs);
    }
    
    for (auto str : tree->structs) {
        proc_struct_definition(str);
    }
}

//
// General functions for iterating over the tree
//

// Global statements
void AstProcessor::proc_global_statement(std::shared_ptr<AstStatement> stmt) {
    switch (stmt->type) {
        case V_AstType::ExternFunc: {} break;
        
        case V_AstType::Func: {
            auto func = std::static_pointer_cast<AstFunction>(stmt);
            proc_block(func->block);
        } break;
        
        default: {}
    }
}

// Structure definitions
void AstProcessor::proc_struct_definition(std::shared_ptr<AstStruct> def) {
    // Nothing used right now
}

// Process blocks
void AstProcessor::proc_block(std::shared_ptr<AstBlock> block) {
    for (auto stmt : block->block) {
        proc_statement(stmt, block);
    }
}

// Process individual statements
void AstProcessor::proc_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block) {
    switch (stmt->type) {
        case V_AstType::Return: {} break;
        case V_AstType::ExprStmt: {} break;
        
        case V_AstType::FuncCallStmt: {
            auto fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
            if (fc->name == "print") {
                proc_print(fc, block);
            }
        } break;
        
        case V_AstType::VarDec: {} break;
        case V_AstType::StructDec: {} break;
        
        case V_AstType::If: {
            auto cond = std::static_pointer_cast<AstIfStmt>(stmt);
            proc_block(cond->true_block);
            proc_block(cond->false_block);
        } break;
        
        case V_AstType::While: {
            auto loop = std::static_pointer_cast<AstWhileStmt>(stmt);
            proc_block(loop->block);
        } break;
        
        case V_AstType::Break: {} break;
        case V_AstType::Continue: {} break;
        
        default: {}
    }
}

void AstProcessor::proc_expression(std::shared_ptr<AstExpression> expr) {
    // Nothing used right now
}

//
// Functions for specific cases
//

// Resolves a print function call
void AstProcessor::proc_print(std::shared_ptr<AstFuncCallStmt> call, std::shared_ptr<AstBlock> block) {
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

