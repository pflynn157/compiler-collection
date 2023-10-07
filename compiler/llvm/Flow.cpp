//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "Compiler.hpp"

// Translates an AST IF statement to LLVM
void Compiler::compileIfStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstIfStmt> condStmt = std::static_pointer_cast<AstIfStmt>(stmt);
    std::shared_ptr<AstBlock> astTrueBlock = condStmt->true_block;
    std::shared_ptr<AstBlock> astFalseBlock = condStmt->false_block;
    
    BasicBlock *trueBlock = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
    BasicBlock *falseBlock = BasicBlock::Create(*context, "false" + std::to_string(blockCount), currentFunc);
    BasicBlock *endBlock = BasicBlock::Create(*context, "end" + std::to_string(blockCount), currentFunc);
    ++blockCount;
    
    logicalOrStack.push(trueBlock);
    logicalAndStack.push(falseBlock);
    
    Value *cond = compileValue(condStmt->expression);
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
        if (stmt2->type == V_AstType::Return) branchEnd = false;
        if (stmt2->type == V_AstType::Break) branchEnd = false;
    }
    if (branchEnd) builder->CreateBr(endBlock);
    
    // False block
    builder->SetInsertPoint(falseBlock);
    branchEnd = true;
    for (auto stmt2 : astFalseBlock->getBlock()) {
        compileStatement(stmt2);
        if (stmt2->type == V_AstType::Return) branchEnd = false;
        if (stmt2->type == V_AstType::Break) branchEnd = false;
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
    Value *cond = compileValue(stmt->expression);
    builder->CreateCondBr(cond, loopBlock, loopEnd);

    builder->SetInsertPoint(loopBlock);
    for (auto stmt : loop->block->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopCmp);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
}

// Translates a for loop to LLVM
void Compiler::compileForStatement(std::shared_ptr<AstStatement> stmt) {
    auto loop = std::static_pointer_cast<AstForStmt>(stmt);
    
    BasicBlock *loopBlock = BasicBlock::Create(*context, "loop_body" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopInc = BasicBlock::Create(*context, "loop_inc" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopCmp = BasicBlock::Create(*context, "loop_cmp" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopEnd = BasicBlock::Create(*context, "loop_end" + std::to_string(blockCount), currentFunc);
    ++blockCount;

    BasicBlock *current = builder->GetInsertBlock();
    loopBlock->moveAfter(current);
    loopInc->moveAfter(loopBlock);
    loopCmp->moveAfter(loopInc);
    loopEnd->moveAfter(loopCmp);
    
    breakStack.push(loopEnd);
    continueStack.push(loopCmp);
    
    // Create the induction variable and back up the symbol tables
    std::map<std::string, AllocaInst *> symtableOld = symtable;
    std::map<std::string, std::shared_ptr<AstDataType>> typeTableOld = typeTable;
    Type *data_type = translateType(loop->data_type);
    
    std::string indexName = loop->index->value;
    AllocaInst *indexVar = builder->CreateAlloca(data_type);
    symtable[indexName] = indexVar;
    typeTable[indexName] = loop->data_type;
    
    Value *startVal = compileValue(loop->start);
    builder->CreateStore(startVal, indexVar);
    
    // Create the rest of the loop
    builder->CreateBr(loopCmp);
    builder->SetInsertPoint(loopCmp);
    
    Value *indexVal = builder->CreateLoad(data_type, indexVar);
    Value *endVal = compileValue(loop->end);
    Value *cond = builder->CreateICmpSLT(indexVal, endVal);
    builder->CreateCondBr(cond, loopBlock, loopEnd);
    
    // Loop increment
    builder->SetInsertPoint(loopInc);
    
    indexVal = builder->CreateLoad(data_type, indexVar);
    Value *incVal = compileValue(loop->step);
    indexVal = builder->CreateAdd(indexVal, incVal);
    builder->CreateStore(indexVal, indexVar);
    
    builder->CreateBr(loopCmp);

    // The body
    builder->SetInsertPoint(loopBlock);
    for (auto stmt : loop->block->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopInc);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
    
    symtable = symtableOld;
    typeTable = typeTableOld;
}

