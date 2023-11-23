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
    
    auto val = run_function(function_map["main"], std::vector<vm_arg_list>());
    return *std::get_if<uint64_t>(&val);
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
        
            // Variable and array declarations
            case V_AstType::VarDec: run_var_decl(ctx, stmt); break;
        
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
            
            // Flow control statements
            case V_AstType::If: run_cond(ctx, stmt); break;
            case V_AstType::While: run_while(ctx, stmt); break;
            
            default: {}
        }
    }
}

//
// Variable and array declarations
//
void AstInterpreter::run_var_decl(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt) {
    auto vd = std::static_pointer_cast<AstVarDec>(stmt);
    
    // Arrays need slightly different treatment
    if (vd->data_type->type == V_AstType::Ptr) {
        auto ptr_type = std::static_pointer_cast<AstPointerType>(vd->data_type);
        ctx->type_map[vd->name] = ptr_type->base_type;
        if (is_int_type(ptr_type->base_type)) {
            ctx->iarray_map[vd->name] = std::vector<uint64_t>();
        } else if (is_float_type(ptr_type->base_type)) {
        
        } else if (is_string_type(ptr_type->base_type)) {
            ctx->sarray_map[vd->name] = std::vector<std::string>();
        }
        
    // Regular scalar variables go right into the normal variable tables
    } else {
        ctx->type_map[vd->name] = vd->data_type;
        if (is_int_type(vd->data_type)) {
            ctx->ivar_map[vd->name] = 0;
        } else if (is_float_type(vd->data_type)) {
        
        } else if (is_string_type(vd->data_type)) {
            ctx->svar_map[vd->name] = "";
        }
    }
}

//
// Runs a conditional statement
//
void AstInterpreter::run_cond(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt) {
    auto cond = std::static_pointer_cast<AstIfStmt>(stmt);
    
    run_iexpression(ctx, cond->expression);
    bool result = (bool)ctx->istack.top();
    ctx->istack.pop();
    
    if (result) run_block(ctx, cond->true_block);
    else run_block(ctx, cond->false_block);
}

//
// Runs a while loop
//
void AstInterpreter::run_while(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt) {
    auto loop = std::static_pointer_cast<AstWhileStmt>(stmt);
    
    while (true) {
        run_iexpression(ctx, loop->expression);
        bool result = (bool)ctx->istack.top();
        ctx->istack.pop();
        if (result == false) break;
        run_block(ctx, loop->block);
    }
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
        case V_AstType::EQ:
        case V_AstType::NEQ:
        case V_AstType::GT:
        case V_AstType::LT:
        case V_AstType::GTE:
        case V_AstType::LTE:
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
        
        case V_AstType::Ptr: {
            auto ptr = std::static_pointer_cast<AstPointerType>(data_type);
            return is_int_type(ptr->base_type);
        }
        
        default: {}
    }
    
    return false;
}

bool AstInterpreter::is_float_type(std::shared_ptr<AstDataType> data_type) {
    switch (data_type->type) {
        case V_AstType::Float32:
        case V_AstType::Float64: return true;
        
        case V_AstType::Ptr: {
            auto ptr = std::static_pointer_cast<AstPointerType>(data_type);
            return is_float_type(ptr->base_type);
        }
        
        default: {}
    }
    
    return false;
}

bool AstInterpreter::is_string_type(std::shared_ptr<AstDataType> data_type) {
    switch (data_type->type) {
        case V_AstType::Char:
        case V_AstType::String: return true;
        
        case V_AstType::Ptr: {
            auto ptr = std::static_pointer_cast<AstPointerType>(data_type);
            return is_string_type(ptr->base_type);
        }
        
        default: {}
    }
    
    return false;
}

//
// Helper functions for determining if a variable is an array of one of the general types
//
bool AstInterpreter::is_int_array(std::shared_ptr<IntrContext> ctx, std::string name) {
    if (ctx->iarray_map.find(name) == ctx->iarray_map.end()) return false;
    return true;
}

bool AstInterpreter::is_float_array(std::shared_ptr<IntrContext> ctx, std::string name) {
    return false;
}

bool AstInterpreter::is_string_array(std::shared_ptr<IntrContext> ctx, std::string name) {
    if (ctx->sarray_map.find(name) == ctx->sarray_map.end()) return false;
    return true;
}

