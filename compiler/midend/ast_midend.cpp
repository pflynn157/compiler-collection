//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
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
    for (int i = 0; i<block->block.size(); i++) {
        auto stmt = block->block[i];
        it_process_statement(stmt, block, i);
    }
}

void AstMidend::it_process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block, int pos) {
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
        
        case V_AstType::BlockStmt: {
            auto stmt2 = std::static_pointer_cast<AstBlockStmt>(stmt);
            process_block_statement(stmt2, block, pos);
            
            it_process_block(stmt2->block);
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
            it_process_expression(stmt2->size, block);
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
            
            //it_process_expression(stmt2->index, block);
            it_process_expression(stmt2->start, block);
            it_process_expression(stmt2->end, block);
            it_process_expression(stmt2->step, block);
            // TODO: data type
            it_process_block(stmt2->block);
        } break;
        
        case V_AstType::ForAll: {
            auto stmt2 = std::static_pointer_cast<AstForAllStmt>(stmt);
            process_forall(stmt2, block);
            
            //it_process_expression(stmt2->index, block);
            //it_process_expression(stmt2->array, block);
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
        it_process_expression(stmt->expression, block);
    }
}

void AstMidend::it_process_expression(std::shared_ptr<AstExpression> &expr, std::shared_ptr<AstBlock> block) {
    if (expr == nullptr) return;

    // Call the public function
    process_expression(expr, block);
    
    // Continue processing
    switch (expr->type) {
        // Operators
        case V_AstType::Neg: break;
        
        case V_AstType::Assign:
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
        case V_AstType::LogicalAnd:
        case V_AstType::LogicalOr: {
            auto binary_op = std::static_pointer_cast<AstBinaryOp>(expr);
            it_process_expression(binary_op->lval, block);
            it_process_expression(binary_op->rval, block);
            std::shared_ptr<AstExpression> expr2 = process_binary_op(binary_op, block);
            if (expr2) {
                expr = expr2;
                return;
            }
            
            if (expr->type == V_AstType::Assign)
                expr2 = process_assign_op(std::static_pointer_cast<AstAssignOp>(expr), block);
            else if (expr->type == V_AstType::Add)
                expr2 = process_add_op(std::static_pointer_cast<AstAddOp>(expr), block);
            else if (expr->type == V_AstType::Sub)
                expr2 = process_sub_op(std::static_pointer_cast<AstSubOp>(expr), block);
            else if (expr->type == V_AstType::Mul)
                expr2 = process_mul_op(std::static_pointer_cast<AstMulOp>(expr), block);
            else if (expr->type == V_AstType::Div)
                expr2 = process_div_op(std::static_pointer_cast<AstDivOp>(expr), block);
            else if (expr->type == V_AstType::Mod)
                expr2 = process_mod_op(std::static_pointer_cast<AstModOp>(expr), block);
            else if (expr->type == V_AstType::And)
                expr2 = process_and_op(std::static_pointer_cast<AstAndOp>(expr), block);
            else if (expr->type == V_AstType::Or)
                expr2 = process_or_op(std::static_pointer_cast<AstOrOp>(expr), block);
            else if (expr->type == V_AstType::Xor)
                expr2 = process_xor_op(std::static_pointer_cast<AstXorOp>(expr), block);
            else if (expr->type == V_AstType::Lsh)
                expr2 = process_lsh_op(std::static_pointer_cast<AstLshOp>(expr), block);
            else if (expr->type == V_AstType::Rsh)
                expr2 = process_rsh_op(std::static_pointer_cast<AstRshOp>(expr), block);
            else if (expr->type == V_AstType::EQ)
                expr2 = process_eq_op(std::static_pointer_cast<AstEQOp>(expr), block);
            else if (expr->type == V_AstType::NEQ)
                expr2 = process_neq_op(std::static_pointer_cast<AstNEQOp>(expr), block);
            else if (expr->type == V_AstType::GT)
                expr2 = process_gt_op(std::static_pointer_cast<AstGTOp>(expr), block);
            else if (expr->type == V_AstType::LT)
                expr2 = process_lt_op(std::static_pointer_cast<AstLTOp>(expr), block);
            else if (expr->type == V_AstType::GTE)
                expr2 = process_gte_op(std::static_pointer_cast<AstGTEOp>(expr), block);
            else if (expr->type == V_AstType::LTE)
                expr2 = process_lte_op(std::static_pointer_cast<AstLTEOp>(expr), block);
            else if (expr->type == V_AstType::LogicalAnd)
                expr2 = process_logical_and_op(std::static_pointer_cast<AstLogicalAndOp>(expr), block);
            else if (expr->type == V_AstType::LogicalOr)
                expr2 = process_logical_or_op(std::static_pointer_cast<AstLogicalOrOp>(expr), block);
            
            if (expr2) {
                expr = expr2;
                return;
            }
        } break;
        
        case V_AstType::Sizeof: break;
        
        // Literals and identifiers
        case V_AstType::CharL: break;
        case V_AstType::IntL: break;
        case V_AstType::FloatL: break;
        case V_AstType::StringL: break;
        case V_AstType::ID: break;
        case V_AstType::ArrayAccess: break;
        case V_AstType::StructAccess: break;
        
        // Expression list
        case V_AstType::ExprList: break;
        
        // Function call expressions
        case V_AstType::FuncCallExpr: break;
        
        // Normally, we shouldn't reach this point
    }
}

