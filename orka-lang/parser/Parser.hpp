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
    bool buildFunctionCallStmt(std::shared_ptr<AstBlock> block, std::string value);
    bool buildReturn(std::shared_ptr<AstBlock> block);
    
    // Variable.cpp
    bool buildVariableDec(std::shared_ptr<AstBlock> block);
    bool build_array_dec(std::shared_ptr<AstBlock> block);
    bool buildVariableAssign(std::shared_ptr<AstBlock> block, std::string value);
    bool buildConst(std::shared_ptr<AstBlock> block, bool isGlobal);
    
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
    bool buildStructMember(std::shared_ptr<AstStruct> str, int tk);
    bool buildStructDec(std::shared_ptr<AstBlock> block);
    bool buildClass();
    bool buildClassDec(std::shared_ptr<AstBlock> block);
    
    // Expression.cpp
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
    bool build_other_token(int tk, bool is_const, std::shared_ptr<ExprContext> ctx) override;
    
    // Parser.cpp
    bool build_import();
    bool buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent = nullptr);
    std::shared_ptr<AstExpression> checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck);
    std::shared_ptr<AstDataType> buildDataType(bool checkBrackets = true);
    std::string getArrayType(std::shared_ptr<AstDataType> dataType);
    void consume_token(token t, std::string message);
private:
    std::map<std::string, AstEnum> enums;
    
    std::shared_ptr<AstClass> currentClass = nullptr;
    std::map<std::string, std::string> classMap;
};

