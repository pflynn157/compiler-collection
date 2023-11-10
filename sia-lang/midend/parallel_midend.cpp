#include <memory>
#include <iostream>

#include <ast/ast_builder.hpp>

#include "parallel_midend.hpp"

ParallelMidend::ParallelMidend(std::shared_ptr<AstTree> tree) {
    this->parse_tree = tree;
    this->tree = std::make_shared<AstTree>(parse_tree->file);
}

void ParallelMidend::run() {
    it_process_block(parse_tree->block, tree->block);
}

void ParallelMidend::it_process_block(std::shared_ptr<AstBlock> &block, std::shared_ptr<AstBlock> &new_block) {
    new_block->mergeSymbols(block);

    for (auto const &stmt : block->block) {
        switch (stmt->type) {
            // Annotated block statements- these are what we want to process
            case V_AstType::BlockStmt: {
                auto block_stmt = std::static_pointer_cast<AstBlockStmt>(stmt);
                process_block_statement(block_stmt, new_block);
            } break;
        
            // Block statements have to be processed separately
            case V_AstType::Func: {
                auto func = std::static_pointer_cast<AstFunction>(stmt);
                auto func2 = std::make_shared<AstFunction>(func->name, func->data_type);
                func2->args = func->args;
                it_process_block(func->block, func2->block);
                new_block->addStatement(func2);
            } break;
        
            // By default, add the statement to the new block
            default: {
                new_block->addStatement(stmt);
            }
        }
    }
}

void ParallelMidend::process_block_statement(std::shared_ptr<AstBlockStmt> &stmt, std::shared_ptr<AstBlock> &block) {
    if (stmt->name == "parallel") {
        auto first = stmt->block->block[0];
    
        auto outlined_func = std::make_shared<AstFunction>("outlined", AstBuilder::buildVoidType());
        tree->block->addStatement(outlined_func);
        
        outlined_func->args.push_back(Var(AstBuilder::buildInt32PointerType(), "global_id"));
        outlined_func->args.push_back(Var(AstBuilder::buildInt32PointerType(), "bound_id"));
        
        // If we have a for statement, do a parallel for loop.
        // Otherwise, we just copy the body
        if (first->type == V_AstType::For) {
            build_omp_parallel_for(outlined_func, first);
        } else {
            for (const auto& stmt2 : stmt->block->block) {
                outlined_func->block->addStatement(stmt2);
            }
        }
        
        auto ret = std::make_shared<AstReturnStmt>();
        outlined_func->block->addStatement(ret);
        
        // Add a call
        auto arg1 = std::make_shared<AstI32>(0);
        auto arg2 = std::make_shared<AstI32>(0);        // no shared arguments
        auto arg3 = std::make_shared<AstFuncRef>("outlined");
        // calling arguments here
        auto args = std::make_shared<AstExprList>();
        args->add_expression(arg1);
        args->add_expression(arg2);
        args->add_expression(arg3);
        
        auto fc = std::make_shared<AstFuncCallStmt>("__kmpc_fork_call");
        fc->expression = args;
        block->addStatement(fc);
    } else {
        for (const auto &stmt2 : stmt->block->block) {
            block->addStatement(stmt2);
        }
    }
}

//
// Builds an OpenMP parallel for statement
//
void ParallelMidend::build_omp_parallel_for(std::shared_ptr<AstFunction> func, std::shared_ptr<AstStatement> first) {
    auto loop = std::static_pointer_cast<AstForStmt>(first);
    //parseBlock(loop->getBlock());
    
    auto indexVd = loop->index;
    auto type = loop->data_type;
    std::string index_name = indexVd->value;
    func->block->addSymbol(index_name, type);

    // Add the following variables
    // 0) The variable declaration for the index variable
    auto idx_vd = std::make_shared<AstVarDec>(indexVd->value, type);
    func->block->addStatement(idx_vd);
    
    // 1) lower = <index variable initial>
    std::string lower_name = "__lower" + std::to_string(index);
    auto lower = std::make_shared<AstVarDec>(lower_name, type);
    func->block->addStatement(lower);
    func->block->addSymbol(lower_name, type);
    
    auto init_lower_expr = std::make_shared<AstAssignOp>();
    init_lower_expr->lval = std::make_shared<AstID>(lower_name);
    init_lower_expr->rval = loop->start;
    auto lowerVA = std::make_shared<AstExprStatement>();
    lowerVA->dataType = type;
    lowerVA->expression = init_lower_expr;
    func->block->addStatement(lowerVA);
    
    // 2) upper = <test expr rval>
    std::string upper_name = "__upper" + std::to_string(index);
    auto upper = std::make_shared<AstVarDec>(upper_name, type);
    func->block->addStatement(upper);
    func->block->addSymbol(upper_name, type);
    
    auto upperAssign = std::make_shared<AstAssignOp>(std::make_shared<AstID>(upper_name), loop->end);
    auto upperVA = std::make_shared<AstExprStatement>();
    upperVA->dataType = type;
    upperVA->expression = upperAssign;
    func->block->addStatement(upperVA);
    
    // 3) stride = <inc val>
    std::string stride_name = "__stride" + std::to_string(index);
    auto stride = std::make_shared<AstVarDec>(stride_name, type);
    func->block->addStatement(stride);
    func->block->addSymbol(stride_name, type);
    
    auto strideAssign = std::make_shared<AstAssignOp>(std::make_shared<AstID>(stride_name), loop->step);
    auto strideVA = std::make_shared<AstExprStatement>();
    strideVA->dataType = type;
    strideVA->expression = strideAssign;
    func->block->addStatement(strideVA);
    
    // 4) last = 0
    std::string last_name = "__last" + std::to_string(index);
    auto last = std::make_shared<AstVarDec>(last_name, type);
    func->block->addStatement(last);
    func->block->addSymbol(last_name, type);
    
    auto lastID = std::make_shared<AstID>(last_name);
    auto lastAssign = std::make_shared<AstAssignOp>(lastID, std::make_shared<AstI32>(0));
    auto lastVA = std::make_shared<AstExprStatement>();
    lastVA->dataType = type;
    lastVA->expression = lastAssign;
    func->block->addStatement(lastVA);
    
    // 5) i <index variable> = lower
    auto index_vd_assign = std::make_shared<AstAssignOp>();
    index_vd_assign->lval = indexVd;
    index_vd_assign->rval = std::make_shared<AstID>(lower_name);
    auto index_vd_expr = std::make_shared<AstExprStatement>();
    index_vd_expr->expression = index_vd_assign;
    func->block->addStatement(index_vd_expr);
    
    auto indexAssign = std::make_shared<AstAssignOp>(std::make_shared<AstID>(index_name), std::make_shared<AstI32>(0));
    auto va = std::make_shared<AstExprStatement>();
    va->dataType = type;
    va->expression = indexAssign;
    func->block->addStatement(va);
    
    // __kmpc_for_static_init_4(0, *global_id, 34, &last, &lower, &upper, &stride, 1, 1);
    auto callArgs1 = std::make_shared<AstExprList>();
    callArgs1->add_expression(std::make_shared<AstI32>(0));
    callArgs1->add_expression(std::make_shared<AstPtrTo>("global_id"));
    callArgs1->add_expression(std::make_shared<AstI32>(34));
    callArgs1->add_expression(std::make_shared<AstRef>(last_name));
    callArgs1->add_expression(std::make_shared<AstRef>(lower_name));
    callArgs1->add_expression(std::make_shared<AstRef>(upper_name));
    callArgs1->add_expression(std::make_shared<AstRef>(stride_name));
    callArgs1->add_expression(std::make_shared<AstI32>(1));
    callArgs1->add_expression(std::make_shared<AstI32>(1));
    
    auto call1 = std::make_shared<AstFuncCallStmt>("__kmpc_for_static_init_4");
    call1->expression = callArgs1;
    func->block->addStatement(call1);
    
    // if (upper > 8) upper = 8;
    auto gt = std::make_shared<AstGTOp>();
    gt->lval = std::make_shared<AstID>(upper_name);
    gt->rval = loop->end;
    auto cond = std::make_shared<AstIfStmt>();
    cond->expression = gt;
    func->block->addStatement(cond);
    
    auto trueBlock = std::make_shared<AstBlock>();
    trueBlock->mergeSymbols(func->block);
    auto upperAssign2 = std::make_shared<AstAssignOp>();
    upperAssign2->lval = std::make_shared<AstID>(upper_name);
    upperAssign2->rval = loop->end;
    auto upperVA2 = std::make_shared<AstExprStatement>();
    upperVA2->dataType = type;
    upperVA2->expression = upperAssign2;
    trueBlock->addStatement(upperVA2);
    cond->true_block = trueBlock;
    cond->false_block = std::make_shared<AstBlock>();
    
    // The loop
    // for (i = lower; i <= upper; i += 1)
    // ==> i = lower
    indexAssign = std::make_shared<AstAssignOp>(std::make_shared<AstID>(index_name), std::make_shared<AstID>(lower_name));
    va = std::make_shared<AstExprStatement>();
    va->dataType = type;
    va->expression = indexAssign;
    func->block->addStatement(va);
    
    // ==> while (i <= upper) { .. i++)
    auto le = std::make_shared<AstLTEOp>();
    le->lval = std::make_shared<AstID>(index_name);
    le->rval = std::make_shared<AstID>(upper_name);
    
    //// Create the loop
    auto block2 = loop->block;
    block2->mergeSymbols(func->block);
    auto whileLoop = std::make_shared<AstWhileStmt>();
    whileLoop->expression = le;
    whileLoop->block = block2;
    func->block->addStatement(whileLoop);
    
    //// Add the increment
    va = std::make_shared<AstExprStatement>();
    va->dataType = type;
    auto inc_add = std::make_shared<AstAddOp>();
    inc_add->lval = std::make_shared<AstID>(index_name);
    inc_add->rval = std::make_shared<AstI32>(1);
    auto inc_assign = std::make_shared<AstAssignOp>(std::make_shared<AstID>(index_name), inc_add);
    va->expression = inc_assign;
    block2->addStatement(va);
    
    // __kmpc_for_static_fini(0, *global_id);
    auto callArgs2 = std::make_shared<AstExprList>();
    callArgs2->add_expression(std::make_shared<AstI32>(0));
    callArgs2->add_expression(std::make_shared<AstPtrTo>("global_id"));
    
    auto call2 = std::make_shared<AstFuncCallStmt>("__kmpc_for_static_fini");
    call2->expression = callArgs2;
    func->block->addStatement(call2);
    
    // Increment the naming variable
    ++index;
}

