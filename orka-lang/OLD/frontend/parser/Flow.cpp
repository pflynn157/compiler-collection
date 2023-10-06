//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <parser/Parser.hpp>
#include <ast.hpp>

// Called if a conditional statement has only one operand. If it does,
// we have to expand to have two operands before we get down to the
// compiler layer
AstExpression *Parser::checkCondExpression(AstExpression *toCheck) {
    AstExpression *expr = toCheck;
    
    switch (toCheck->getType()) {
        case AstType::ID: {
            AstID *id = static_cast<AstID *>(toCheck);
            DataType dataType = typeMap[id->getValue()].first;
            
            AstEQOp *eq = new AstEQOp;
            eq->setLVal(id);
            
            switch (dataType) {
                case DataType::Bool: eq->setRVal(new AstBool(1)); break;
                case DataType::Byte:
                case DataType::UByte: eq->setRVal(new AstByte(1)); break;
                case DataType::Short:
                case DataType::UShort: eq->setRVal(new AstWord(1)); break;
                case DataType::Int32:
                case DataType::UInt32: eq->setRVal(new AstInt(1)); break;
                case DataType::Int64:
                case DataType::UInt64: eq->setRVal(new AstQWord(1)); break;
                
                default: {}
            }
            
            expr = eq;
        } break;
        
        default: {}
    }
    
    return expr;
}

// Builds a conditional statement
bool Parser::buildConditional(AstBlock *block) {
    AstIfStmt *cond = new AstIfStmt;
    if (!buildExpression(cond, DataType::Void, Then)) return false;
    block->addStatement(cond);
    
    AstExpression *expr = checkCondExpression(cond->getExpressions().at(0));
    cond->clearExpressions();
    cond->addExpression(expr);

    ++layer;
    buildBlock(cond->getBlockStmt(), layer, cond);
    
    return true;
}

// Builds an ELIF statement
bool Parser::buildElif(AstIfStmt *block) {
    AstElifStmt *elif = new AstElifStmt;
    if (!buildExpression(elif, DataType::Void, Then)) return false;
    block->addBranch(elif);
    
    AstExpression *expr = checkCondExpression(elif->getExpressions().at(0));
    elif->clearExpressions();
    elif->addExpression(expr);
    
    buildBlock(elif->getBlockStmt(), layer, block, true);
    return true;
}

// Builds an ELSE statement
bool Parser::buildElse(AstIfStmt *block) {
    AstElseStmt *elsee = new AstElseStmt;
    block->addBranch(elsee);
    
    buildBlock(elsee->getBlockStmt(), layer);
    return true;
}

// Builds a while statement
bool Parser::buildWhile(AstBlock *block) {
    AstWhileStmt *loop = new AstWhileStmt;
    if (!buildExpression(loop, DataType::Void, Do)) return false;
    block->addStatement(loop);
    
    AstExpression *expr = checkCondExpression(loop->getExpressions().at(0));
    loop->clearExpressions();
    loop->addExpression(expr);
    
    ++layer;
    buildBlock(loop->getBlockStmt(), layer);
    
    return true;
}

// Builds an infinite loop statement
bool Parser::buildRepeat(AstBlock *block) {
    AstRepeatStmt *loop = new AstRepeatStmt;
    block->addStatement(loop);
    
    ++layer;
    buildBlock(loop->getBlockStmt(), layer);

    return true;
}

// Builds a for loop
bool Parser::buildFor(AstBlock *block) {
    AstForStmt *loop = new AstForStmt;
    block->addStatement(loop);
    
    // Get the index
    Token token = scanner->getNext();
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected variable name for index.");
        return false;
    }
    
    loop->setIndex(new AstID(token.id_val));
    
    token = scanner->getNext();
    if (token.type != In) {
        syntax->addError(scanner->getLine(), "Expected \"in\".");
        return false;
    }
    
    // Build the starting and ending expression
    if (!buildExpression(loop, DataType::Void, Do, Range)) return false;
    
    if (loop->getExpressionCount() == 2) {
        AstExpression *end = loop->getExpressions().back();
        AstExpression *start = loop->getExpressions().front();
        
        loop->clearExpressions();
        
        loop->setStartBound(start);
        loop->setEndBound(end);
    } else {
        syntax->addError(scanner->getLine(), "Invalid expression in for loop.");
        return false;
    }
    
    ++layer;
    buildBlock(loop->getBlockStmt(), layer);
    
    return true;
}

// Builds a forall loop
bool Parser::buildForAll(AstBlock *block) {
    AstForAllStmt *loop = new AstForAllStmt;
    block->addStatement(loop);
    
    // Get the index
    Token token = scanner->getNext();
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected variable name for index.");
        return false;
    }
    
    loop->setIndex(new AstID(token.id_val));
    
    token = scanner->getNext();
    if (token.type != In) {
        syntax->addError(scanner->getLine(), "Expected \"in\".");
        return false;
    }
    
    // Get the array we are iterating through
    token = scanner->getNext();
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected array for iteration value.");
        return false;
    }
    
    loop->setArray(new AstID(token.id_val));
    
    // Make sure we end with the "do" keyword
    token = scanner->getNext();
    if (token.type != Do) {
        syntax->addError(scanner->getLine(), "Expected \"do\".");
        return false;
    }
    
    ++layer;
    buildBlock(loop->getBlockStmt(), layer);
    
    return true;
}

// Builds a loop keyword
bool Parser::buildLoopCtrl(AstBlock *block, bool isBreak) {
    if (isBreak) block->addStatement(new AstBreak);
    else block->addStatement(new AstContinue);
    
    Token token = scanner->getNext();
    if (token.type != SemiColon) {
        syntax->addError(scanner->getLine(), "Expected \';\' after break or continue.");
        return false;
    }
    
    return true;
}

