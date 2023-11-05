#include "ast_midend.hpp"

AstMidend::AstMidend(std::shared_ptr<AstTree> tree) {
    this->tree = tree;
}

//
// Called to run any midend
//
void AstMidend::run() {
    it_process_block(tree->block);
}

void AstMidend::it_process_block(std::shared_ptr<AstBlock> block) {
    // Call the public function
    process_block(block);

    // Continue processing
    for (const auto &stmt : block->block) {
        it_process_statement(stmt, block);
    }
}

void AstMidend::it_process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block) {
    // Call the public function
    process_statement(stmt, block);
    
    // Continue processing, and call all public functions
    switch (stmt->type) {
        case V_AstType::ExternFunc: {
            auto stmt2 = std::static_pointer_cast<AstExternFunction>(stmt);
            process_extern_function(stmt2, block);
        } break;
        
        case V_AstType::Func: {
            auto stmt2 = std::static_pointer_cast<AstFunction>(stmt);
            process_function(stmt2, block);
            
            // Internal processing
            it_process_block(stmt2->block);
            // TODO: Types
        } break;
        
        case V_AstType::Return: {
            auto stmt2 = std::static_pointer_cast<AstReturnStmt>(stmt);
            process_return(stmt2, block);
        } break;
        
        case V_AstType::ExprStmt: {
            auto stmt2 = std::static_pointer_cast<AstExprStatement>(stmt);
            process_expr_statement(stmt2, block);
        } break;
        
        case V_AstType::FuncCallStmt: {
            auto stmt2 = std::static_pointer_cast<AstFuncCallStmt>(stmt);
            process_function_call(stmt2, block);
        } break;
        
        case V_AstType::VarDec: {
            auto stmt2 = std::static_pointer_cast<AstVarDec>(stmt);
            process_var_decl(stmt2, block);
            
            // Internal processing
            it_process_expression(stmt2->size);
            // TODO: Types
        } break;
        
        case V_AstType::StructDec: {
            auto stmt2 = std::static_pointer_cast<AstStructDec>(stmt);
            process_struct_decl(stmt2, block);
        } break;
        
        case V_AstType::If: {
            auto stmt2 = std::static_pointer_cast<AstIfStmt>(stmt);
            process_conditional(stmt2, block);
            
            it_process_block(stmt2->true_block);
            it_process_block(stmt2->false_block);
        } break;
        
        case V_AstType::While: {
            auto stmt2 = std::static_pointer_cast<AstWhileStmt>(stmt);
            process_while(stmt2, block);
            
            it_process_block(stmt2->block);
        } break;
        
        case V_AstType::Repeat: {
            auto stmt2 = std::static_pointer_cast<AstRepeatStmt>(stmt);
            process_repeat(stmt2, block);
            
            it_process_block(stmt2->block);
        } break;
        
        case V_AstType::For: {
            auto stmt2 = std::static_pointer_cast<AstForStmt>(stmt);
            process_for(stmt2, block);
            
            it_process_expression(stmt2->index);
            it_process_expression(stmt2->start);
            it_process_expression(stmt2->end);
            it_process_expression(stmt2->step);
            // TODO: data type
            it_process_block(stmt2->block);
        } break;
        
        case V_AstType::ForAll: {
            auto stmt2 = std::static_pointer_cast<AstForAllStmt>(stmt);
            process_forall(stmt2, block);
            
            it_process_expression(stmt2->index);
            it_process_expression(stmt2->array);
            //TODO: data type
            it_process_block(stmt2->block);
        } break;
        
        case V_AstType::Break: {
            auto stmt2 = std::static_pointer_cast<AstBreak>(stmt);
            process_break(stmt2, block);
        } break;
        
        case V_AstType::Continue: {
            auto stmt2 = std::static_pointer_cast<AstContinue>(stmt);
            process_continue(stmt2, block);
        } break;
        
        // In theory, this should never happen
        default: {}
    }
    
    // Call internal processing on any expressions
    if (stmt->hasExpression()) {
        it_process_expression(stmt->expression);
    }
}

void AstMidend::it_process_expression(std::shared_ptr<AstExpression> expr) {
    // Call the public function
    process_expression(expr);
}

