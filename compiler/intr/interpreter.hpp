#pragma once

#include <string>
#include <memory>
#include <map>
#include <stack>
#include <cstdint>

#include <ast/ast.hpp>

//
// This contains the contextual information
// Every function creates a context
//
// * var_map -> Holds variable values
// * stack -> Holds values from expression evaluation
//
struct IntrContext {
    std::map<std::string, std::shared_ptr<AstDataType>> type_map;
    std::shared_ptr<AstDataType> func_type;
    
    // For variable storage
    std::map<std::string, int> ivar_map;
    std::map<std::string, std::string> svar_map;
    
    // For expression evaluation
    std::stack<uint64_t> istack;
    std::stack<double> fstack;
    std::stack<std::string> sstack;
};

//
// This handles running the actual interpreter
//
struct AstInterpreter {
    explicit AstInterpreter(std::shared_ptr<AstTree> tree);
    int run();
    
    int run_function(std::shared_ptr<AstFunction> func, std::vector<uint64_t> args);
    void call_function(std::shared_ptr<IntrContext> ctx, std::string name, std::shared_ptr<AstExprList> args);
    void run_block(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstBlock> block);
    void run_expression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> type);
    void run_iexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    void run_fexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    void run_sexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    void run_print(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExprList> args);
    std::shared_ptr<AstDataType> interpret_type(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    bool is_int_type(std::shared_ptr<AstDataType> data_type);
    bool is_float_type(std::shared_ptr<AstDataType> data_type);
    bool is_string_type(std::shared_ptr<AstDataType> data_type);
    
protected:
    std::shared_ptr<AstTree> tree;
    std::map<std::string, std::shared_ptr<AstFunction>> function_map;
};

