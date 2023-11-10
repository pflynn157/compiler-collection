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
        auto outlined_func = std::make_shared<AstFunction>("outlined", AstBuilder::buildVoidType());
        //tree->block->block.insert(tree->block->block.begin(), outlined_func);
        tree->block->addStatement(outlined_func);
        
        for (const auto& stmt2 : stmt->block->block) {
            outlined_func->block->addStatement(stmt2);
        }
        
        auto ret = std::make_shared<AstReturnStmt>();
        outlined_func->block->addStatement(ret);
        
        // Add a call
        auto fc = std::make_shared<AstFuncCallStmt>("outlined");
        fc->expression = std::make_shared<AstExprList>();
        block->addStatement(fc);
    } else {
        for (const auto &stmt2 : stmt->block->block) {
            block->addStatement(stmt2);
        }
    }
}

