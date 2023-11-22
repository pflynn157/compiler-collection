#include <iostream>
#include <cstdarg>

#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

#include "interpreter.hpp"

//
// Initializes the source-level interpreter
//
AstInterpreter::AstInterpreter(std::shared_ptr<AstTree> tree) {
    this->tree = tree;
}

//
// The entry point of the interpreter
//
// The first step is to catalog all our functions, and then run
// the main function.
//
int AstInterpreter::run() {
    for (auto const& stmt : tree->block->block) {
        if (stmt->type != V_AstType::Func) continue;
        
        auto func = std::static_pointer_cast<AstFunction>(stmt);
        function_map[func->name] = func;
    }
    
    // Verify we have the main function
    if (function_map.find("main") == function_map.end()) {
        std::cout << "[FATAL] Unable to find main function." << std::endl;
        return 1;
    }
    
    return run_function(function_map["main"], std::vector<uint64_t>());
}

//
// For running functions
//
int AstInterpreter::run_function(std::shared_ptr<AstFunction> func, std::vector<uint64_t> args) {   
    auto ctx = std::make_shared<IntrContext>();
    ctx->func_type = func->data_type;
    
    // Merge arguments into the symbol table
    for (int i = 0; i<func->args.size(); i++) {
        auto arg = func->args[i];
        ctx->type_map[arg.name] = arg.type;
        
        // TODO: Check type
        if (is_int_type(arg.type)) {
            ctx->ivar_map[arg.name] = args[i];
        } else if (is_float_type(arg.type)) {
        
        } else if (is_string_type(arg.type)) {
            std::string value = *(std::string*)(args[i]);
            ctx->svar_map[arg.name] = value;
        }
    }
    
    // Run the block
    run_block(ctx, func->block);
    
    // At the end, check the stack
    if (ctx->istack.empty()) return 0;
    return ctx->istack.top();
}

void AstInterpreter::call_function(std::shared_ptr<IntrContext> ctx, std::string name, std::shared_ptr<AstExprList> args) {
    auto func = function_map[name];
    std::vector<uint64_t> addrs;
    
    // TODO: Check type
    for (int i = 0; i<args->list.size(); i++) {
        auto arg = args->list[i];
        auto data_type = func->args[i].type;
        if (is_int_type(data_type)) {
            run_iexpression(ctx, arg);
            uint64_t value = ctx->istack.top();
            ctx->istack.pop();
            addrs.push_back(value);
        } else if (is_float_type(data_type)) {
        
        } else if (is_string_type(data_type)) {
            run_sexpression(ctx, arg);
            std::string value = ctx->sstack.top();
            ctx->sstack.pop();
            addrs.push_back((uint64_t)&value);
        }
    }
    
    // Run it
    run_function(func, addrs);
}

//
// Runs a block of statements
//
void AstInterpreter::run_block(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstBlock> block) {
    // TODO: Create symbol table within the context
    
    // Run the block
    for (auto const &stmt : block->block) {
        switch (stmt->type) {
            // Expression statements
            case V_AstType::ExprStmt: {
                auto stmt2 = std::static_pointer_cast<AstExprStatement>(stmt);
                run_expression(ctx, stmt2->expression, stmt2->dataType);
            } break;
        
            // Variable declarations
            case V_AstType::VarDec: {
                auto vd = std::static_pointer_cast<AstVarDec>(stmt);
                // TODO: Check type
                ctx->ivar_map[vd->name] = 0;
                ctx->type_map[vd->name] = vd->data_type;
            } break;
        
            // Return statements
            case V_AstType::Return: {
                if (!stmt->hasExpression()) break;
                run_expression(ctx, stmt->expression, ctx->func_type);
            } break;
            
            // Function calls
            case V_AstType::FuncCallStmt: {
                auto fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
                if (fc->name == "print") {
                    run_print(ctx, std::static_pointer_cast<AstExprList>(fc->expression));
                } else {
                    auto args = std::static_pointer_cast<AstExprList>(fc->expression);
                    call_function(ctx, fc->name, args);
                }
            } break;
            
            default: {}
        }
    }
}

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
            ctx->istack.push(ctx->ivar_map[id->value]);
        } break;
        
        // Assign operator
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstAssignOp>(expr);
            run_iexpression(ctx, op->rval);
            
            switch (op->lval->type) {
                // Simple variables
                case V_AstType::ID: {
                    auto id = std::static_pointer_cast<AstID>(op->lval);
                    ctx->ivar_map[id->value] = ctx->istack.top();
                    ctx->istack.pop();
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
        
        case V_AstType::StringL: {
            auto s = std::static_pointer_cast<AstString>(expr);
            ctx->sstack.push(s->value);
        } break;
        
        // Variables
        case V_AstType::ID: {
            auto id = std::static_pointer_cast<AstID>(expr);
            ctx->sstack.push(ctx->svar_map[id->value]);
        } break;
        
        // Assign operator
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstAssignOp>(expr);
            run_sexpression(ctx, op->rval);
            
            switch (op->lval->type) {
                // Simple variables
                case V_AstType::ID: {
                    auto id = std::static_pointer_cast<AstID>(op->lval);
                    ctx->svar_map[id->value] = ctx->sstack.top();
                    ctx->sstack.pop();
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

//
// Runs the builtin print call
//
void AstInterpreter::run_print(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExprList> args) {
    for (auto const &arg : args->list) {
        switch (arg->type) {
            // Print a string literal
            case V_AstType::StringL: {
                auto s = std::static_pointer_cast<AstString>(arg);
                std::cout << s->value;
            } break;
            
            // Print an integer literal
            case V_AstType::IntL: {
                auto i = std::static_pointer_cast<AstInt>(arg);
                std::cout << i->value;
            } break;
            
            // Identifier
            case V_AstType::ID: {
                auto id = std::static_pointer_cast<AstID>(arg);
                auto data_type = ctx->type_map[id->value];
                if (is_int_type(data_type)) {
                    std::cout << ctx->ivar_map[id->value];
                } else if (is_float_type(data_type)) {
                
                } else if (is_string_type(data_type)) {
                    std::cout << ctx->svar_map[id->value];
                }
            } break;
            
            // Print a binary operation
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
            {
                auto data_type = interpret_type(ctx, arg);
                if (data_type == nullptr) {
                    std::cout << "[ERR:<UNK_TYPE>]";
                    break;
                }
                
                run_expression(ctx, arg, data_type);
                if (data_type->type == V_AstType::Int32) {
                    std::cout << ctx->istack.top();
                    ctx->istack.pop();
                }
                // TODO: Other types here
            } break;
            
            default: {}
        }
    }
    
    std::cout << std::endl;
}

//
// Needed in some places where types are not explicity known
// Note: We only return either Int32, String, or Float32, since these generalize
// to one of the three overall types are expression works with.
//
// Generally, we decide on types based on the lval
//
std::shared_ptr<AstDataType> AstInterpreter::interpret_type(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {
    switch (expr->type) {
        case V_AstType::IntL: return AstBuilder::buildInt32Type();
        
        case V_AstType::ID: {
            auto id = std::static_pointer_cast<AstID>(expr);
            return ctx->type_map[id->value];
        }
        
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
        {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            auto d_type = interpret_type(ctx, op->lval);
            if (d_type) return d_type;
            d_type = interpret_type(ctx, op->rval);
            if (d_type) return d_type;
            return nullptr;
        }
        
        default: {}
    }
    
    return nullptr;
}

//
// Helper functions for determining the three general type
//
bool AstInterpreter::is_int_type(std::shared_ptr<AstDataType> data_type) {
    switch (data_type->type) {
        case V_AstType::Bool:
        case V_AstType::Int8:
        case V_AstType::Int16:
        case V_AstType::Int32:
        case V_AstType::Int64: return true;
        
        default: {}
    }
    
    return false;
}

bool AstInterpreter::is_float_type(std::shared_ptr<AstDataType> data_type) {
    switch (data_type->type) {
        case V_AstType::Float32:
        case V_AstType::Float64: return true;
        
        default: {}
    }
    
    return false;
}

bool AstInterpreter::is_string_type(std::shared_ptr<AstDataType> data_type) {
    switch (data_type->type) {
        case V_AstType::Char:
        case V_AstType::String: return true;
        
        default: {}
    }
    
    return false;
}

