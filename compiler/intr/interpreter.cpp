#include <iostream>

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
    
    return run_function(function_map["main"], std::make_shared<AstExprList>());
}

//
// For running functions
//
int AstInterpreter::run_function(std::shared_ptr<AstFunction> func, std::shared_ptr<AstExprList> args) {
    auto ctx = std::make_shared<IntrContext>();
    ctx->func_type = func->data_type;
    run_block(ctx, func->block);
    
    // At the end, check the stack
    if (ctx->istack.empty()) return 0;
    return ctx->istack.top();
}

//
// Runs a block of statements
//
void AstInterpreter::run_block(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstBlock> block) {
    // TODO: Create symbol table within the context
    
    // Run the block
    for (auto const &stmt : block->block) {
        switch (stmt->type) {
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
    switch (type->type) {
        case V_AstType::Bool:
        case V_AstType::Int8:
        case V_AstType::Int16:
        case V_AstType::Int32:
        case V_AstType::Int64: run_iexpression(ctx, expr); break;
        
        case V_AstType::Float32:
        case V_AstType::Float64: run_fexpression(ctx, expr); break;
        
        case V_AstType::Char:
        case V_AstType::String: run_sexpression(ctx, expr); break;
        
        default: {}
    }
}

// Runs an integer-based expression
void AstInterpreter::run_iexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {
    switch (expr->type) {
        // Constants
        case V_AstType::IntL: {
            auto i = std::static_pointer_cast<AstInt>(expr);
            ctx->istack.push(i->value);
        } break;
        
        // Operators
        case V_AstType::Add:
        {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            run_iexpression(ctx, op->lval);
            run_iexpression(ctx, op->rval);
            
            uint64_t rval = ctx->istack.top();
            ctx->istack.pop();
            uint64_t lval = ctx->istack.top();
            ctx->istack.pop();
            
            if (expr->type == V_AstType::Add) ctx->istack.push(lval + rval);
        } break;
        
        default: {}
    }
}

// Runs a floating point expression
void AstInterpreter::run_fexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {

}

// Runs a string expression
void AstInterpreter::run_sexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr) {

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
            
            // Print a binary operation
            case V_AstType::Add:
            {
                auto data_type = interpret_type(arg);
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
std::shared_ptr<AstDataType> AstInterpreter::interpret_type(std::shared_ptr<AstExpression> expr) {
    switch (expr->type) {
        case V_AstType::IntL: return AstBuilder::buildInt32Type();
        
        case V_AstType::Add:
        {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            auto d_type = interpret_type(op->lval);
            if (d_type) return d_type;
            d_type = interpret_type(op->rval);
            if (d_type) return d_type;
            return nullptr;
        }
        
        default: {}
    }
    
    return nullptr;
}

