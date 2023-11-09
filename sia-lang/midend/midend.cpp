#include <memory>
#include <iostream>

#include <ast/ast_builder.hpp>

#include "midend.hpp"

void Midend::process_block_statement(std::shared_ptr<AstBlockStmt> stmt, std::shared_ptr<AstBlock> block, int pos) {
    block->block.erase(block->block.begin() + pos);
    
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
        //auto fc = std::make_shared<AstFuncCallStmt>("outlined");
        //block->block.insert(block->block.begin() + pos, fc);
    }
}

