#pragma once

#include <string>
#include <stack>

#include <ast/ast.hpp>
#include <parser/ErrorManager.hpp>

struct ExprContext {
    std::stack<std::shared_ptr<AstExpression>> output;
    std::stack<std::shared_ptr<AstOp>> opStack;
    std::shared_ptr<AstDataType> varType;
    bool lastWasOp = true;
};

//
// The base for parsers in our compiler toolchain
//
class BaseParser {
public:
    explicit BaseParser(std::string input);
    
    virtual bool parse() { return false; }
    
    std::shared_ptr<AstTree> getTree() { return tree; }
    
    // Expression parsers
    std::shared_ptr<AstExpression> checkExpression(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> varType);
    bool applyHigherPred(std::shared_ptr<ExprContext> ctx);
    bool applyAssoc(std::shared_ptr<ExprContext> ctx);
protected:
    std::string input = "";
    std::shared_ptr<AstTree> tree;
    std::shared_ptr<ErrorManager> syntax;
};

