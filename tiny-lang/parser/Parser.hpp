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
    bool buildFunction(int startToken, std::string className = "");
    bool buildFunctionCallStmt(std::shared_ptr<AstBlock> block, std::string fc_name);
    bool buildReturn(std::shared_ptr<AstBlock> block);
    
    // Variable.cpp
    bool buildVariableDec(std::shared_ptr<AstBlock> block);
    bool build_array_dec(std::shared_ptr<AstBlock> block);
    bool buildVariableAssign(std::shared_ptr<AstBlock> block, std::string var_name);
    bool buildConst(std::shared_ptr<AstBlock> block, bool isGlobal);
    
    // Flow.cpp
    bool buildConditional(std::shared_ptr<AstBlock> block);
    bool buildWhile(std::shared_ptr<AstBlock> block);
    bool buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak);
    
    // Structure.cpp
    bool buildStruct();
    bool buildStructMember(std::shared_ptr<AstStruct> str, int tk);
    bool buildStructDec(std::shared_ptr<AstBlock> block);
    
    // Expression.cpp
    // Override
    bool is_constant(int tk) override;
    bool is_id(int tk) override;
    bool is_operator(int tk) override;
    bool is_sub_expr_start(int tk) override;
    bool is_sub_expr_end(int tk) override;
    bool is_list_delim(int tk) override;
    int get_sub_expr_end() override;
    
    bool build_operator(int tk, std::shared_ptr<ExprContext> ctx) override;
    std::shared_ptr<AstExpression> build_constant(int tk) override;
    bool build_identifier(std::shared_ptr<AstBlock> block, int tk, std::shared_ptr<ExprContext> ctx) override;
    
    // Parser.cpp
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    std::shared_ptr<AstDataType> buildDataType(bool checkBrackets = true);
    void consume_token(token expected, std::string msg);
private:
    //std::unique_ptr<Lex> lex;
};

