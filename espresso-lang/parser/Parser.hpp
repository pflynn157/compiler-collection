//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <memory>

#include <lex/Lex.hpp>
#include <parser/base_parser.hpp>
#include <parser/ErrorManager.hpp>
#include <ast/ast.hpp>

// The parser class
// The parser is in charge of performing all parsing and AST-building tasks
// It is also in charge of the error manager

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    
    bool parse();
    
    void debugScanner();
protected:
    // Function.cpp
    bool getFunctionArgs(std::vector<Var> &args, std::shared_ptr<AstBlock> block);
    bool buildFunction(std::shared_ptr<AstBlock> block, Token startToken);
    bool buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken, Token varToken);
    bool buildReturn(std::shared_ptr<AstBlock> block);
    
    // Variable.cpp
    bool buildVariableDec(std::shared_ptr<AstBlock> block);
    bool buildVariableAssign(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildConst(std::shared_ptr<AstBlock> block, bool isGlobal);
    
    // Flow.cpp
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    bool buildConditional(std::shared_ptr<AstBlock> block);
    bool buildWhile(std::shared_ptr<AstBlock> block);
    bool buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak);
    
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> buildExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        TokenType stopToken = SemiColon, TokenType separateToken = EmptyToken,
                        bool isConst = false);
private:
    std::unique_ptr<Scanner> scanner;
};

