//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <memory>

#include <lex/Lex.hpp>
#include <parser/ErrorManager.hpp>
#include <ast/ast.hpp>
#include <parser/base_parser.hpp>

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
    bool buildFunction(Token startToken, std::string className = "");
    bool buildFunctionCallStmt(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildReturn(std::shared_ptr<AstBlock> block);
    
    // Variable.cpp
    bool buildVariableDec(std::shared_ptr<AstBlock> block);
    bool buildVariableAssign(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildArrayAssign(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildConst(bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(std::shared_ptr<AstBlock> block);
    bool buildWhile(std::shared_ptr<AstBlock> block);
    bool buildRepeat(std::shared_ptr<AstBlock> block);
    bool buildFor(std::shared_ptr<AstBlock> block);
    bool buildForAll(std::shared_ptr<AstBlock> block);
    bool buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak);
    
    // Structure.cpp
    bool buildEnum();
    bool buildStruct();
    bool buildStructMember(std::shared_ptr<AstStruct> str, Token token);
    bool buildStructDec(std::shared_ptr<AstBlock> block);
    bool buildStructAssign(std::shared_ptr<AstBlock> block, Token idToken);
    bool buildClass();
    bool buildClassDec(std::shared_ptr<AstBlock> block);
    
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> buildExpression(
                        std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        TokenType stopToken = SemiColon,
                        bool isConst = false, bool buildList = false);
    std::shared_ptr<AstExpression> checkExpression(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> varType);
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    int isConstant(std::string name);
private:
    std::unique_ptr<Scanner> scanner;
    std::shared_ptr<AstClass> currentClass = nullptr;
    
    std::map<std::string, std::string> classMap;
    std::map<std::string, AstEnum> enums;
};

