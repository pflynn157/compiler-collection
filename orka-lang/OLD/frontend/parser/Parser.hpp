//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>

#include <lex/Lex.hpp>
#include <error/Manager.hpp>
#include <ast.hpp>

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
    bool getFunctionArgs(std::vector<Var> &args);
    bool buildFunction(Token startToken, std::string className = "");
    bool buildFunctionCallStmt(AstBlock *block, Token idToken);
    bool buildReturn(AstBlock *block);
    
    // Variable.cpp
    bool buildVariableDec(AstBlock *block);
    bool buildVariableAssign(AstBlock *block, Token idToken);
    bool buildArrayAssign(AstBlock *block, Token idToken);
    bool buildConst(bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(AstBlock *block);
    bool buildElif(AstIfStmt *block);
    bool buildElse(AstIfStmt *block);
    bool buildWhile(AstBlock *block);
    bool buildRepeat(AstBlock *block);
    bool buildFor(AstBlock *block);
    bool buildForAll(AstBlock *block);
    bool buildLoopCtrl(AstBlock *block, bool isBreak);
    
    // Structure.cpp
    bool buildEnum();
    bool buildStruct();
    bool buildStructMember(AstStruct *str, Token token);
    bool buildStructDec(AstBlock *block);
    bool buildStructAssign(AstBlock *block, Token idToken);
    bool buildClass();
    bool buildClassDec(AstBlock *block);
    
    bool buildBlock(AstBlock *block, int stopLayer = 0, AstIfStmt *parentBlock = nullptr, bool inElif = false);
    bool buildExpression(AstStatement *stmt, DataType currentType,
                        TokenType stopToken = SemiColon, TokenType separateToken = EmptyToken,
                        AstExpression **dest = nullptr, bool isConst = false);
    AstExpression *checkExpression(AstExpression *expr, DataType varType);
    AstExpression *checkCondExpression(AstExpression *toCheck);
    int isConstant(std::string name);
private:
    std::string input = "";
    Scanner *scanner;
    AstTree *tree;
    ErrorManager *syntax;
    int layer = 0;
    AstClass *currentClass = nullptr;
    
    std::map<std::string, std::pair<DataType,DataType>> typeMap;
    std::map<std::string, std::string> classMap;
    std::map<std::string, std::pair<DataType, AstExpression*>> globalConsts;
    std::map<std::string, std::pair<DataType, AstExpression*>> localConsts;
    std::map<std::string, EnumDec> enums;
};

