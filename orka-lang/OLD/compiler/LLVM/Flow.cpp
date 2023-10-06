//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <LLVM/Compiler.hpp>

// Translates an AST IF statement to LLVM
void Compiler::compileIfStatement(AstStatement *stmt) {
    AstIfStmt *condStmt = static_cast<AstIfStmt *>(stmt);
    bool hasBranches = condStmt->getBranches().size();

    BasicBlock *trueBlock = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
    BasicBlock *falseBlock = nullptr;
    BasicBlock *endBlock = BasicBlock::Create(*context, "end" + std::to_string(blockCount), currentFunc);
    if (hasBranches) falseBlock = BasicBlock::Create(*context, "false" + std::to_string(blockCount), currentFunc);
    ++blockCount;

    Value *cond = compileValue(stmt->getExpressions().at(0));
    if (hasBranches) builder->CreateCondBr(cond, trueBlock, falseBlock);
    else builder->CreateCondBr(cond, trueBlock, endBlock);

    // Align the blocks
    BasicBlock *current = builder->GetInsertBlock();
    trueBlock->moveAfter(current);

    if (hasBranches) {
        falseBlock->moveAfter(trueBlock);
        endBlock->moveAfter(falseBlock);
    } else {
        endBlock->moveAfter(trueBlock);
    }

    builder->SetInsertPoint(trueBlock);
    bool hasBreak = false;
    for (auto stmt : condStmt->getBlock()) {
        compileStatement(stmt);
        if (stmt->getType() == AstType::Break) hasBreak = true;
    }
    if (!hasBreak) builder->CreateBr(endBlock);

    // Branches
    bool hadElif = false;
    bool hadElse = false;

    for (auto stmt : condStmt->getBranches()) {
        if (stmt->getType() == AstType::Elif) {
            AstElifStmt *elifStmt = static_cast<AstElifStmt *>(stmt);
            
            BasicBlock *trueBlock2 = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
            BasicBlock *falseBlock2 = BasicBlock::Create(*context, "false" + std::to_string(blockCount), currentFunc);
            
            // Align
            if (!hadElif) builder->SetInsertPoint(falseBlock);
            BasicBlock *current = builder->GetInsertBlock();
            trueBlock2->moveAfter(current);
            falseBlock2->moveAfter(trueBlock2);
            
            Value *cond = compileValue(stmt->getExpressions().at(0));
            builder->CreateCondBr(cond, trueBlock2, falseBlock2);
            
            builder->SetInsertPoint(trueBlock2);
            bool hasBreak = false;
            for (auto stmt2 : elifStmt->getBlock()) {
                compileStatement(stmt2);
            }
            if (!hasBreak) builder->CreateBr(endBlock);
            
            builder->SetInsertPoint(falseBlock2);
            hadElif = true;
        } else if (stmt->getType() == AstType::Else) {
            AstElseStmt *elseStmt = static_cast<AstElseStmt *>(stmt);
            
            if (!hadElif) builder->SetInsertPoint(falseBlock);
            
            bool hasBreak = false;
            for (auto stmt2 : elseStmt->getBlock()) {
                compileStatement(stmt2);
            }
            if (!hasBreak) builder->CreateBr(endBlock);
            
            hadElse = true;
        }
    }

    if (hadElif && !hadElse) {
        builder->CreateBr(endBlock);
    }

    // Start at the end block
    builder->SetInsertPoint(endBlock);
}

// Translates a while statement to LLVM
void Compiler::compileWhileStatement(AstStatement *stmt) {
    AstWhileStmt *loop = static_cast<AstWhileStmt *>(stmt);

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
    Value *cond = compileValue(stmt->getExpressions().at(0));
    builder->CreateCondBr(cond, loopBlock, loopEnd);

    builder->SetInsertPoint(loopBlock);
    for (auto stmt : loop->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopCmp);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
}

// Translates a repeat statement to LLVM
void Compiler::compileRepeatStatement(AstStatement *stmt) {
    AstRepeatStmt *loop = static_cast<AstRepeatStmt *>(stmt);
    
    BasicBlock *loopBlock = BasicBlock::Create(*context, "loop_body" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopEnd = BasicBlock::Create(*context, "loop_end" + std::to_string(blockCount), currentFunc);
    ++blockCount;
    
    BasicBlock *current = builder->GetInsertBlock();
    loopBlock->moveAfter(current);
    loopEnd->moveAfter(loopBlock);
    
    breakStack.push(loopEnd);
    continueStack.push(loopBlock);
    
    builder->CreateBr(loopBlock);
    builder->SetInsertPoint(loopBlock);
    
    for (auto stmt : loop->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopBlock);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
}

// Translates a for loop to LLVM
void Compiler::compileForStatement(AstStatement *stmt) {
    AstForStmt *loop = static_cast<AstForStmt *>(stmt);
    
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
    std::map<std::string, DataType> typeTableOld = typeTable;
    
    std::string indexName = loop->getIndex()->getValue();
    AllocaInst *indexVar = builder->CreateAlloca(Type::getInt32Ty(*context));
    symtable[indexName] = indexVar;
    typeTable[indexName] = DataType::Int32;
    
    Value *startVal = compileValue(loop->getStartBound());
    builder->CreateStore(startVal, indexVar);
    
    // Create the rest of the loop
    builder->CreateBr(loopCmp);
    builder->SetInsertPoint(loopCmp);
    
    Value *indexVal = builder->CreateLoad(indexVar);
    Value *endVal = compileValue(loop->getEndBound());
    Value *cond = builder->CreateICmpSLT(indexVal, endVal);
    builder->CreateCondBr(cond, loopBlock, loopEnd);
    
    // Loop increment
    builder->SetInsertPoint(loopInc);
    
    indexVal = builder->CreateLoad(indexVar);
    Value *incVal = compileValue(loop->getStep());
    indexVal = builder->CreateAdd(indexVal, incVal);
    builder->CreateStore(indexVal, indexVar);
    
    builder->CreateBr(loopCmp);

    // The body
    builder->SetInsertPoint(loopBlock);
    for (auto stmt : loop->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopInc);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
    
    symtable = symtableOld;
    typeTable = typeTableOld;
}

// Translates a for-all loop to LLVM
void Compiler::compileForAllStatement(AstStatement *stmt) {
    AstForAllStmt *loop = static_cast<AstForAllStmt *>(stmt);
    
    // Setup the blocks
    BasicBlock *loopLoad = BasicBlock::Create(*context, "loop_load" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopBody = BasicBlock::Create(*context, "loop_body" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopInc = BasicBlock::Create(*context, "loop_inc" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopCmp = BasicBlock::Create(*context, "loop_cmp" + std::to_string(blockCount), currentFunc);
    BasicBlock *loopEnd = BasicBlock::Create(*context, "loop_end" + std::to_string(blockCount), currentFunc);
    ++blockCount;

    BasicBlock *current = builder->GetInsertBlock();
    loopLoad->moveAfter(current);
    loopBody->moveAfter(loopLoad);
    loopInc->moveAfter(loopBody);
    loopCmp->moveAfter(loopInc);
    loopEnd->moveAfter(loopCmp);
    
    breakStack.push(loopEnd);
    continueStack.push(loopCmp);
    
    ///
    // Create the induction variable, the max-size variable, and the element variables
    //
    std::map<std::string, AllocaInst *> symtableOld = symtable;
    std::map<std::string, DataType> typeTableOld = typeTable;
    
    // The induction variable
    std::string arrayName = loop->getArray()->getValue();
    std::string indexName = loop->getIndex()->getValue();
    DataType indexType1 = ptrTable[arrayName];
    
    Type *indexType = translateType(indexType1);
    AllocaInst *indexVar = builder->CreateAlloca(indexType);
    symtable[indexName] = indexVar;
    typeTable[indexName] = indexType1;
    
    AllocaInst *inductionVar = builder->CreateAlloca(Type::getInt32Ty(*context));
    builder->CreateStore(builder->getInt32(0), inductionVar);
    
    // The size value
    AllocaInst *arrayPtr = symtable[arrayName];
    Value *sizePtr = builder->CreateStructGEP(arrayPtr, 1);
    Value *sizeVal = builder->CreateLoad(sizePtr);
    
    ///
    // Create the loop comparison
    //
    builder->CreateBr(loopCmp);
    builder->SetInsertPoint(loopCmp);
    
    Value *inductionVarVal = builder->CreateLoad(inductionVar);
    Value *cond = builder->CreateICmpSLT(inductionVarVal, sizeVal);
    builder->CreateCondBr(cond, loopLoad, loopEnd);
    
    ///
    // Loop increment
    //
    builder->SetInsertPoint(loopInc);
    
    inductionVarVal = builder->CreateLoad(inductionVar);
    inductionVarVal = builder->CreateAdd(inductionVarVal, builder->getInt32(1));
    builder->CreateStore(inductionVarVal, inductionVar);
    
    builder->CreateBr(loopCmp);
    
    ///
    // Loop load-> loads the next element from the array
    //
    builder->SetInsertPoint(loopLoad);
    
    inductionVarVal = builder->CreateLoad(inductionVar);
    
    Value *arrayStructPtr = builder->CreateStructGEP(arrayPtr, 0);
    Value *arrayLoad = builder->CreateLoad(arrayStructPtr);
    Value *ep = builder->CreateGEP(arrayLoad, inductionVarVal);
    Value *epLd = builder->CreateLoad(ep);
    builder->CreateStore(epLd, indexVar);
    
    builder->CreateBr(loopBody);
    
    ///
    // Loop body
    //
    builder->SetInsertPoint(loopBody);
    for (auto stmt : loop->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopInc);
    
    builder->SetInsertPoint(loopEnd);
    
    breakStack.pop();
    continueStack.pop();
    
    symtable = symtableOld;
    typeTable = typeTableOld;
}
