//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <map>
#include <stack>
#include <memory>

#include <parser/base_parser.hpp>
#include <parser/ErrorManager.hpp>
#include <ast/ast.hpp>
#include <lex/lex.hpp>

// The parser class
// The parser is in charge of performing all parsing and AST-building tasks
// It is also in charge of the error manager
class Parser : public BaseParser {
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
    bool buildConst(std::shared_ptr<AstBlock> block, bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(std::shared_ptr<AstBlock> block);
    bool buildWhile(std::shared_ptr<AstBlock> block);
    bool buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak);
    
    // Structure.cpp
    bool buildStruct();
    bool buildStructMember(std::shared_ptr<AstStruct> str, Token tk);
    bool buildStructDec(std::shared_ptr<AstBlock> block);
    
    // Expression.cpp
    std::shared_ptr<AstExpression> buildConstExpr(Token tk);
    bool buildOperator(Token tk, std::shared_ptr<ExprContext> ctx);
    bool buildIDExpr(std::shared_ptr<AstBlock> block, Token tk, std::shared_ptr<ExprContext> ctx);
    std::shared_ptr<AstExpression> buildExpression(
                        std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        TokenType stopToken = t_semicolon,
                        bool isConst = false, bool buildList = false);
    
    // Parser.cpp
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    std::shared_ptr<AstDataType> buildDataType(bool checkBrackets = true);
private:
    std::unique_ptr<Scanner> scanner;
};

