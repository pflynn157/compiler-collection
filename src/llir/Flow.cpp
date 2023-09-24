//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "Compiler.hpp"

// Translates an AST IF statement to LLVM
void Compiler::compileIfStatement(std::shared_ptr<AstStatement> stmt) {
    /*auto condStmt = std::static_pointer_cast<AstIfStmt>(stmt);
    bool hasBranches = condStmt->getBranches().size();

    LLIR::Block *trueBlock = new LLIR::Block("true" + std::to_string(blockCount));
    LLIR::Block *falseBlock = nullptr;
    LLIR::Block *endBlock = new LLIR::Block("end" + std::to_string(blockCount));
    
    // The break stack pushes are for the logical boolean expressions
    logicalOrStack.push(trueBlock);
    if (hasBranches) {
        falseBlock = new LLIR::Block("false" + std::to_string(blockCount));
        logicalAndStack.push(falseBlock);
    } else {
        logicalAndStack.push(endBlock);
    }
    ++blockCount;

    LLIR::Operand *cond = compileValue(stmt->getExpression(), V_AstType::Void, trueBlock);
    if (hasBranches) builder->createBr(falseBlock);
    else builder->createBr(endBlock);
    
    logicalAndStack.pop();
    logicalOrStack.pop();

    // Align the blocks
    LLIR::Block *current = builder->getInsertPoint();
    builder->addBlockAfter(current, trueBlock);

    if (hasBranches) {
        builder->addBlockAfter(trueBlock, falseBlock);
        builder->addBlockAfter(falseBlock, endBlock);
    } else {
        builder->addBlockAfter(trueBlock, endBlock);
    }

    builder->setInsertPoint(trueBlock);
    bool hasBreak = false;
    bool hasEndingRet = false;
    for (auto stmt : condStmt->getBlock()) {
        compileStatement(stmt);
        if (stmt->getType() == AstType::Break) hasBreak = true;
    }
    if (condStmt->getBlock().back()->getType() == AstType::Return) hasEndingRet = true;
    if (!hasBreak && !hasEndingRet) builder->createBr(endBlock);

    // Branches
    bool hadElif = false;
    bool hadElse = false;
    int subCount = 0;

    for (auto stmt : condStmt->getBranches()) {
        if (stmt->getType() == AstType::Elif) {
            AstElifStmt *elifStmt = static_cast<AstElifStmt *>(stmt);
            
            LLIR::Block *trueBlock2 = new LLIR::Block(std::to_string(subCount) + "true" + std::to_string(blockCount));
            LLIR::Block *falseBlock2 = new LLIR::Block(std::to_string(subCount) + "false" + std::to_string(blockCount));
            ++subCount;
            
            logicalAndStack.push(falseBlock2);
            logicalOrStack.push(trueBlock2);
            
            // Align
            if (!hadElif) builder->setInsertPoint(falseBlock);
            LLIR::Block *current = builder->getInsertPoint();
            builder->addBlockAfter(current, trueBlock2);
            builder->addBlockAfter(trueBlock2, falseBlock2);
            
            LLIR::Operand *cond = compileValue(stmt->getExpression(), V_AstType::Void, trueBlock2);
            builder->createBr(falseBlock2);
            
            logicalAndStack.pop();
            logicalOrStack.pop();
            
            builder->setInsertPoint(trueBlock2);
            bool hasBreak = false;
            bool hasEndingRet = false;
            for (auto stmt2 : elifStmt->getBlock()) {
                compileStatement(stmt2);
            }
            if (elifStmt->getBlock().back()->getType() == AstType::Return) hasEndingRet = true;
            if (!hasBreak && !hasEndingRet) builder->createBr(endBlock);
            
            builder->setInsertPoint(falseBlock2);
            hadElif = true;
        } else if (stmt->getType() == AstType::Else) {
            AstElseStmt *elseStmt = static_cast<AstElseStmt *>(stmt);
            
            if (!hadElif) builder->setInsertPoint(falseBlock);
            
            bool hasBreak = false;
            bool hasEndingRet = false;
            for (auto stmt2 : elseStmt->getBlock()) {
                compileStatement(stmt2);
            }
            if (elseStmt->getBlock().back()->getType() == AstType::Return) hasEndingRet = true;
            if (!hasBreak && !hasEndingRet) builder->createBr(endBlock);
            
            hadElse = true;
        }
    }

    if (hadElif && !hadElse) {
        builder->createBr(endBlock);
    }

    // Start at the end block
    builder->setInsertPoint(endBlock);*/
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

