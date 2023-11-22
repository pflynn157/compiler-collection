#include <iostream>

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
        case V_AstType::IntL: {
            auto i = std::static_pointer_cast<AstInt>(expr);
            ctx->istack.push(i->value);
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

}

