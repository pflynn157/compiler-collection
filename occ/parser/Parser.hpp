//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <stack>
#include <memory>

#include <parser/ErrorManager.hpp>
#include <ast/ast.hpp>
#include <lex/lex.hpp>

// The parser class
// The parser is in charge of performing all parsing and AST-building tasks
// It is also in charge of the error manager

class Parser {
public:
    explicit Parser(std::string input);
    ~Parser();
    
    bool parse();
    
    std::shared_ptr<AstTree> getTree() { return tree; }
    
    void debugScanner();
protected:
    // Function.cpp
    bool getFunctionArgs(std::shared_ptr<AstBlock> block, std::vector<Var> &args);
    bool buildFunction(Token startToken, std::string className = "");
    bool buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildReturn(std::shared_ptr<AstBlock> block);
    
    // Variable.cpp
    bool buildVariableDec(std::shared_ptr<AstBlock> block);
    bool buildVariableAssign(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildConst(bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(std::shared_ptr<AstBlock> block);
    bool buildWhile(std::shared_ptr<AstBlock> block);
    bool buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak);
    
    // Structure.cpp
    bool buildStruct();
    bool buildStructMember(std::shared_ptr<AstStruct> str, Token tk);
    bool buildStructDec(std::shared_ptr<AstBlock> block);
    
    // Expression.cpp
    struct ExprContext {
        std::stack<std::shared_ptr<AstExpression>> output;
        std::stack<std::shared_ptr<AstOp>> opStack;
        std::shared_ptr<AstDataType> varType;
        bool lastWasOp = true;
    };
    
    std::shared_ptr<AstExpression> buildConstExpr(Token tk);
    bool buildOperator(Token tk, std::shared_ptr<ExprContext> ctx);
    bool buildIDExpr(std::shared_ptr<AstBlock> block, Token tk, std::shared_ptr<ExprContext> ctx);
    bool applyHigherPred(std::shared_ptr<ExprContext> ctx);
    bool applyAssoc(std::shared_ptr<ExprContext> ctx);
    std::shared_ptr<AstExpression> buildExpression(
                        std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        TokenType stopToken = t_semicolon,
                        bool isConst = false, bool buildList = false);
    std::shared_ptr<AstExpression> checkExpression(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> varType);
    
    // Parser.cpp
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    int isConstant(std::string name);
    bool isVar(std::string name);
    bool isFunc(std::string name);
    std::shared_ptr<AstDataType> buildDataType(bool checkBrackets = true);
private:
    std::string input = "";
    std::unique_ptr<Scanner> scanner;
    std::shared_ptr<AstTree> tree;
    std::shared_ptr<ErrorManager> syntax;
    
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> globalConsts;
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> localConsts;
    std::vector<std::string> funcs;
};

