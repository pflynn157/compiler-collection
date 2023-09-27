//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "Compiler.hpp"

// Translates an AST IF statement to LLVM
void Compiler::compileIfStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstIfStmt> condStmt = std::static_pointer_cast<AstIfStmt>(stmt);
    std::shared_ptr<AstBlock> astTrueBlock = condStmt->true_block;
    std::shared_ptr<AstBlock> astFalseBlock = condStmt->false_block;
    
    LLIR::Block *trueBlock = new LLIR::Block("true" + std::to_string(blockCount));
    LLIR::Block *falseBlock = new LLIR::Block("false" + std::to_string(blockCount));
    LLIR::Block *endBlock = new LLIR::Block("end" + std::to_string(blockCount));
    ++blockCount;
    
    logicalOrStack.push(trueBlock);
    logicalAndStack.push(falseBlock);
    
    std::shared_ptr<AstDataType> dtype = std::make_shared<AstDataType>(V_AstType::Void);
    LLIR::Operand *cond = compileValue(stmt->expression, dtype, trueBlock);
    builder->createBr(falseBlock);
    
    logicalAndStack.pop();
    logicalOrStack.pop();
    
    // Align the blocks
    LLIR::Block *current = builder->getInsertPoint();
    builder->addBlockAfter(current, trueBlock);
    builder->addBlockAfter(trueBlock, falseBlock);
    builder->addBlockAfter(falseBlock, endBlock);
    
    // True block
    builder->setInsertPoint(trueBlock);
    bool branchEnd = true;
    for (auto stmt2 : astTrueBlock->getBlock()) {
        compileStatement(stmt2);
        if (stmt2->type == V_AstType::Return) branchEnd = false;
        if (stmt2->type == V_AstType::Break) branchEnd = false;
    }
    if (branchEnd) builder->createBr(endBlock);
    
    // False block
    builder->setInsertPoint(falseBlock);
    branchEnd = true;
    for (auto stmt2 : astFalseBlock->getBlock()) {
        compileStatement(stmt2);
        if (stmt2->type == V_AstType::Return) branchEnd = false;
        if (stmt2->type == V_AstType::Break) branchEnd = false;
    }
    if (branchEnd) builder->createBr(endBlock);
    
    // End block
    builder->setInsertPoint(endBlock);
}

// Translates a while statement to LLVM
void Compiler::compileWhileStatement(std::shared_ptr<AstStatement> stmt) {
    auto loop = std::static_pointer_cast<AstWhileStmt>(stmt);
    
    LLIR::Block *loopBlock = new LLIR::Block("loop_body" + std::to_string(blockCount));
    LLIR::Block *loopCmp = new LLIR::Block("loop_cmp" + std::to_string(blockCount));
    LLIR::Block *loopEnd = new LLIR::Block("loop_end" + std::to_string(blockCount));
    ++blockCount;
    
    LLIR::Block *current = builder->getInsertPoint();
    builder->addBlockAfter(current, loopBlock);
    builder->addBlockAfter(loopBlock, loopCmp);
    builder->addBlockAfter(loopCmp, loopEnd);
    
    breakStack.push(loopEnd);
    continueStack.push(loopCmp);
    
    builder->createBr(loopCmp);
    builder->setInsertPoint(loopCmp);
    LLIR::Operand *cond = compileValue(stmt->expression, nullptr, loopBlock);
    builder->createBr(loopEnd);
    
    builder->setInsertPoint(loopBlock);
    for (auto stmt : loop->block->getBlock()) {
        compileStatement(stmt);
    }
    builder->createBr(loopCmp);
    
    builder->setInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
}

