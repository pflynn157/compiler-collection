//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "Compiler.hpp"

// Translates an AST IF statement to LLVM
void Compiler::compileIfStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstIfStmt> condStmt = std::static_pointer_cast<AstIfStmt>(stmt);
    std::shared_ptr<AstBlock> astTrueBlock = condStmt->getTrueBlock();
    std::shared_ptr<AstBlock> astFalseBlock = condStmt->getFalseBlock();
    
    BasicBlock *trueBlock = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
    BasicBlock *falseBlock = BasicBlock::Create(*context, "false" + std::to_string(blockCount), currentFunc);
    BasicBlock *endBlock = BasicBlock::Create(*context, "end" + std::to_string(blockCount), currentFunc);
    ++blockCount;
    
    logicalOrStack.push(trueBlock);
    logicalAndStack.push(falseBlock);
    
    Value *cond = compileValue(condStmt->getExpression());
    builder->CreateCondBr(cond, trueBlock, falseBlock);
    
    logicalAndStack.pop();
    logicalOrStack.pop();
    
    BasicBlock *current = builder->GetInsertBlock();
    trueBlock->moveAfter(current);
    falseBlock->moveAfter(trueBlock);
    endBlock->moveAfter(falseBlock);
    
    // True block
    builder->SetInsertPoint(trueBlock);
    bool branchEnd = true;
    for (auto stmt2 : astTrueBlock->getBlock()) {
        compileStatement(stmt2);
        if (stmt2->getType() == V_AstType::Return) branchEnd = false;
        if (stmt2->getType() == V_AstType::Break) branchEnd = false;
    }
    if (branchEnd) builder->CreateBr(endBlock);
    
    // False block
    builder->SetInsertPoint(falseBlock);
    branchEnd = true;
    for (auto stmt2 : astFalseBlock->getBlock()) {
        compileStatement(stmt2);
        if (stmt2->getType() == V_AstType::Return) branchEnd = false;
        if (stmt2->getType() == V_AstType::Break) branchEnd = false;
    }
    if (branchEnd) builder->CreateBr(endBlock);
    
    // End block
    builder->SetInsertPoint(endBlock);
}

// Translates a while statement to LLVM
void Compiler::compileWhileStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstWhileStmt> loop = std::static_pointer_cast<AstWhileStmt>(stmt);

    BasicBlock *loopBlock = BasicBlock::Create(*context, "loop_body" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopCmp = BasicBlock::Create(*context, "loop_cmp" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopEnd = BasicBlock::Create(*context, "loop_end" + std::to_string(blockCount), currentFunc);
    ++blockCount;

    BasicBlock *current = builder->GetInsertBlock();
    loopBlock->moveAfter(current);
    loopCmp->moveAfter(loopBlock);
    loopEnd->moveAfter(loopCmp);
    
    breakStack.push(loopEnd);
    continueStack.push(loopCmp);

    builder->CreateBr(loopCmp);
    builder->SetInsertPoint(loopCmp);
    Value *cond = compileValue(stmt->getExpression());
    builder->CreateCondBr(cond, loopBlock, loopEnd);

    builder->SetInsertPoint(loopBlock);
    for (auto stmt : loop->getBlock()->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopCmp);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
}

