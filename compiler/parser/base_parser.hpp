//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <stack>

#include <ast/ast.hpp>
#include <parser/ErrorManager.hpp>
#include <parser/base_lex.hpp>

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
    
    // These should be overriden by the language parser for the universal parser to work
    virtual bool is_constant(int tk) { return false; }
    virtual bool is_id(int tk) { return false; }
    virtual bool is_operator(int tk) { return false; }
    virtual bool is_sub_expr_start(int tk) { return false; }
    virtual bool is_sub_expr_end(int tk) { return false; }
    virtual int get_sub_expr_end() { return 0; }
    virtual bool is_list_delim(int tk) { return false; }
    virtual bool build_operator(int tk, std::shared_ptr<ExprContext> ctx) { return false; }
    virtual bool build_identifier(std::shared_ptr<AstBlock> block, int tk, std::shared_ptr<ExprContext> ctx) { return false; }
    virtual std::shared_ptr<AstExpression> build_constant(int tk) { return nullptr; }
    virtual bool build_other_token(int tk, bool is_const, std::shared_ptr<ExprContext> ctx) { return false; }
    
    // Expression parsers
    std::shared_ptr<AstExpression> checkExpression(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> varType);
    bool applyHigherPred(std::shared_ptr<ExprContext> ctx);
    bool applyAssoc(std::shared_ptr<ExprContext> ctx);
    void post_process_operator(std::shared_ptr<ExprContext> ctx, std::shared_ptr<AstBinaryOp> op, std::shared_ptr<AstUnaryOp> op1, bool is_unary);
    std::shared_ptr<AstExpression> buildExpression(
                        std::shared_ptr<AstBlock> block, std::shared_ptr<AstDataType> currentType,
                        int stopToken,
                        bool isConst = false, bool buildList = false);
protected:
    std::string input = "";
    std::shared_ptr<AstTree> tree;
    std::shared_ptr<ErrorManager> syntax;
    std::unique_ptr<BaseLex> lex;
};

