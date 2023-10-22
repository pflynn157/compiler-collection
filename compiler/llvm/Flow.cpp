//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
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

// Translates a repeat statement to LLVM
void Compiler::compileRepeatStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstRepeatStmt> loop = std::static_pointer_cast<AstRepeatStmt>(stmt);
    
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
    
    for (auto stmt : loop->block->getBlock()) {
        compileStatement(stmt);
    }
    builder->CreateBr(loopBlock);
    
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

// Translates a for-all loop to LLVM
void Compiler::compileForAllStatement(std::shared_ptr<AstStatement> stmt) {
    std::shared_ptr<AstForAllStmt> loop = std::static_pointer_cast<AstForAllStmt>(stmt);
    
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
    
    //
    // Get the structure type for the array- will be needed later on
    //
    std::string arrayName = loop->array->value;
    std::string indexName = loop->index->value;
    Type *indexType = translateType(loop->data_type);
    
    std::string strTypeName = structVarTable[arrayName];
    StructType *strType = structTable[strTypeName];             //struct
    Type *elementType = structElementTypeTable[strTypeName][0];     //*i32
    Type *sizeType = structElementTypeTable[strTypeName][1];        //i32
    
    ///
    // Create the induction variable, the max-size variable, and the element variables
    //
    std::map<std::string, AllocaInst *> symtableOld = symtable;
    std::map<std::string, std::shared_ptr<AstDataType>> typeTableOld = typeTable;
    
    // The induction variable
    AllocaInst *indexVar = builder->CreateAlloca(indexType);
    symtable[indexName] = indexVar;
    typeTable[indexName] = loop->data_type;
    
    Type *idxType = Type::getInt32Ty(*context);
    AllocaInst *inductionVar = builder->CreateAlloca(idxType);
    builder->CreateStore(builder->getInt32(0), inductionVar);
    
    // The size value
    AllocaInst *arrayPtr = symtable[arrayName];
    
    PointerType *strTypePtr = PointerType::getUnqual(strType);
    Value *ptr = builder->CreateLoad(strTypePtr, arrayPtr);
    
    Value *sizePtr = builder->CreateStructGEP(strType, ptr, 1);
    Value *sizeVal = builder->CreateLoad(indexType, sizePtr);
    
    ///
    // Create the loop comparison
    //
    builder->CreateBr(loopCmp);
    builder->SetInsertPoint(loopCmp);
    
    Value *inductionVarVal = builder->CreateLoad(indexType, inductionVar);
    Value *cond = builder->CreateICmpSLT(inductionVarVal, sizeVal);
    builder->CreateCondBr(cond, loopLoad, loopEnd);
    
    ///
    // Loop increment
    //
    builder->SetInsertPoint(loopInc);
    
    inductionVarVal = builder->CreateLoad(indexType, inductionVar);
    inductionVarVal = builder->CreateAdd(inductionVarVal, builder->getInt32(1));
    builder->CreateStore(inductionVarVal, inductionVar);
    
    builder->CreateBr(loopCmp);
    
    ///
    // Loop load-> loads the next element from the array
    //
    builder->SetInsertPoint(loopLoad);
    
    inductionVarVal = builder->CreateLoad(indexType, inductionVar);
    
    Value *arrayStructPtr = builder->CreateStructGEP(strType, ptr, 0);
    Value *arrayLoad = builder->CreateLoad(elementType, arrayStructPtr);
    Value *ep = builder->CreateGEP(indexType, arrayLoad, inductionVarVal);
    Value *epLd = builder->CreateLoad(indexType, ep);
    builder->CreateStore(epLd, indexVar);
    
    builder->CreateBr(loopBody);
    
    ///
    // Loop body
    //
    builder->SetInsertPoint(loopBody);
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

