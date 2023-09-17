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

extern "C" {
#include <lex/lex.h>
}

// The parser class
// The parser is in charge of performing all parsing and AST-building tasks
// It is also in charge of the error manager

class Parser {
public:
    explicit Parser(std::string input);
    ~Parser();
    
    bool parse();
    
    AstTree *getTree() { return tree; }
    
    void debugScanner();
protected:
    // Function.cpp
    bool getFunctionArgs(AstBlock *block, std::vector<Var> &args);
    bool buildFunction(token startToken, std::string className = "");
    bool buildFunctionCallStmt(AstBlock *block, token idToken);
    bool buildReturn(AstBlock *block);
    
    // Variable.cpp
    bool buildVariableDec(AstBlock *block);
    bool buildVariableAssign(AstBlock *block, token idToken);
    bool buildConst(bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(AstBlock *block);
    bool buildWhile(AstBlock *block);
    bool buildLoopCtrl(AstBlock *block, bool isBreak);
    
    // Structure.cpp
    bool buildStruct();
    bool buildStructMember(AstStruct *str, token tk);
    bool buildStructDec(AstBlock *block);
    
    // Expression.cpp
    struct ExprContext {
        std::stack<std::shared_ptr<AstExpression>> output;
        std::stack<std::shared_ptr<AstOp>> opStack;
        AstDataType *varType;
        bool lastWasOp = true;
    };
    
    std::shared_ptr<AstExpression> buildConstExpr(token tk);
    bool buildOperator(token tk, ExprContext *ctx);
    bool buildIDExpr(AstBlock *block, token tk, ExprContext *ctx);
    bool applyHigherPred(ExprContext *ctx);
    bool applyAssoc(ExprContext *ctx);
    std::shared_ptr<AstExpression> buildExpression(
                        AstBlock *block, AstDataType *currentType,
                        token stopToken = t_semicolon,
                        bool isConst = false, bool buildList = false);
    std::shared_ptr<AstExpression> checkExpression(std::shared_ptr<AstExpression> expr, AstDataType *varType);
    
    bool buildBlock(AstBlock *block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> checkCondExpression(AstBlock *block, std::shared_ptr<AstExpression> toCheck);
    int isConstant(std::string name);
    bool isVar(std::string name);
    bool isFunc(std::string name);
    AstDataType *buildDataType(bool checkBrackets = true);
private:
    std::string input = "";
    //Scanner *scanner;
    lex *scanner;
    AstTree *tree;
    ErrorManager *syntax;
    
    std::map<std::string, std::pair<AstDataType *, std::shared_ptr<AstExpression>>> globalConsts;
    std::map<std::string, std::pair<AstDataType *, std::shared_ptr<AstExpression>>> localConsts;
    std::vector<std::string> funcs;
};

