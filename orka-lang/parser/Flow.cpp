//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <parser/Parser.hpp>
#include <ast/ast.hpp>

// Called if a conditional statement has only one operand. If it does,
// we have to expand to have two operands before we get down to the
// compiler layer
std::shared_ptr<AstExpression> Parser::checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck) {
    std::shared_ptr<AstExpression> expr = toCheck;
    
    switch (toCheck->type) {
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(toCheck);
            std::shared_ptr<AstDataType> dataType = block->getDataType(id->value);            
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->lval = id;
            
            switch (dataType->type) {
                case V_AstType::Bool: eq->rval = std::make_shared<AstI32>(1); break;
                case V_AstType::Int8: eq->rval = std::make_shared<AstI8>(1); break;
                case V_AstType::Int16: eq->rval = std::make_shared<AstI16>(1); break;
                case V_AstType::Int32: eq->rval = std::make_shared<AstI32>(1); break;
                case V_AstType::Int64: eq->rval = std::make_shared<AstI64>(1); break;
                
                default: {}
            }
            
            expr = eq;
        } break;
        
        case V_AstType::I32L: {
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->lval = expr;
            eq->rval = std::make_shared<AstI32>(1);
            expr = eq;
        } break;
        
        default: {}
    }
    
    return expr;
}

// Builds a conditional statement
bool Parser::buildConditional(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstIfStmt> cond = std::make_shared<AstIfStmt>();
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, Then);
    if (!arg) return false;
    cond->expression = arg;
    block->addStatement(cond);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, cond->expression);
    cond->expression = expr;
    
    std::shared_ptr<AstBlock> true_block = std::make_shared<AstBlock>();
    true_block->mergeSymbols(block);
    cond->true_block = true_block;
    
    std::shared_ptr<AstBlock> false_block = std::make_shared<AstBlock>();
    false_block->mergeSymbols(block);
    cond->false_block = false_block;
    buildBlock(true_block, cond);
    
    return true;
}

// Builds a while statement
bool Parser::buildWhile(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstWhileStmt> loop = std::make_shared<AstWhileStmt>();
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, Do);
    if (!arg) return false;
    loop->expression = arg;
    block->addStatement(loop);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, loop->expression);
    loop->expression = expr;
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->block = block2;
    
    return true;
}

// Builds an infinite loop statement
bool Parser::buildRepeat(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstRepeatStmt> loop = std::make_shared<AstRepeatStmt>();
    block->addStatement(loop);
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->block = block2;

    return true;
}

// Builds a for loop
bool Parser::buildFor(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstForStmt> loop = std::make_shared<AstForStmt>();
    block->addStatement(loop);
    
    // Get the index
    Token token = scanner->getNext();
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected variable name for index.");
        return false;
    }
    
    loop->indexVar = std::make_shared<AstID>(token.id_val);
    
    token = scanner->getNext();
    if (token.type != In) {
        syntax->addError(scanner->getLine(), "Expected \"in\".");
        return false;
    }
    
    // Build the starting and ending expression
    // TODO: This will need to be fixed
    /*auto bounds_expr = buildExpression(block, nullptr, Do, Range);
    if (bounds_expr == nullptr) return false;
    loop->expression = bounds_expr;
    
    if (loop->getExpressionCount() == 2) {
        AstExpression *end = loop->getExpressions().back();
        AstExpression *start = loop->getExpressions().front();
        
        loop->clearExpressions();
        
        loop->setStartBound(start);
        loop->setEndBound(end);
    } else {
        syntax->addError(scanner->getLine(), "Invalid expression in for loop.");
        return false;
    }*/
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->block = block2;
    
    return true;
}

// Builds a forall loop
bool Parser::buildForAll(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstForAllStmt> loop = std::make_shared<AstForAllStmt>();
    block->addStatement(loop);
    
    // Get the index
    Token token = scanner->getNext();
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected variable name for index.");
        return false;
    }
    
    loop->indexVar = std::make_shared<AstID>(token.id_val);
    
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
    
    auto array_id = std::make_shared<AstID>(token.id_val);
    loop->arrayVar = array_id;
    
    // Make sure we end with the "do" keyword
    token = scanner->getNext();
    if (token.type != Do) {
        syntax->addError(scanner->getLine(), "Expected \"do\".");
        return false;
    }
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->block = block2;
    
    return true;
}

// Builds a loop keyword
bool Parser::buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak) {
    if (isBreak) block->addStatement(std::make_shared<AstBreak>());
    else block->addStatement(std::make_shared<AstContinue>());
    
    Token tk = scanner->getNext();
    if (tk.type != SemiColon) {
        syntax->addError(0, "Expected \';\' after break or continue.");
        return false;
    }
    
    return true;
}

