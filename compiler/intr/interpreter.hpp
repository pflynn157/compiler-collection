#pragma once

#include <string>
#include <memory>
#include <map>
#include <stack>
#include <vector>
#include <cstdint>
#include <variant>

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
    
    // For array storage
    std::map<std::string, std::vector<uint64_t>> iarray_map;
    std::map<std::string, std::vector<std::string>> sarray_map;
    
    // For expression evaluation
    std::stack<uint64_t> istack;
    std::stack<double> fstack;
    std::stack<std::string> sstack;
};

//
// For passing arguments
//
typedef std::variant<uint64_t, float, std::string, std::vector<uint64_t>, std::vector<float>, std::vector<std::string>> vm_arg_list;

//
// This handles running the actual interpreter
//
struct AstInterpreter {
    explicit AstInterpreter(std::shared_ptr<AstTree> tree);
    int run();
    
    // function.cpp
    vm_arg_list run_function(std::shared_ptr<AstFunction> func, std::vector<vm_arg_list> args);
    vm_arg_list call_function(std::shared_ptr<IntrContext> ctx, std::string name, std::shared_ptr<AstExprList> args);
    void run_print(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExprList> args);
    
    // interpreter.cpp
    void run_block(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstBlock> block);
    void run_var_decl(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt);
    void run_cond(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt);
    void run_while(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstStatement> stmt);
    std::shared_ptr<AstDataType> interpret_type(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    bool is_int_type(std::shared_ptr<AstDataType> data_type);
    bool is_float_type(std::shared_ptr<AstDataType> data_type);
    bool is_string_type(std::shared_ptr<AstDataType> data_type);
    bool is_int_array(std::shared_ptr<IntrContext> ctx, std::string name);
    bool is_float_array(std::shared_ptr<IntrContext> ctx, std::string name);
    bool is_string_array(std::shared_ptr<IntrContext> ctx, std::string name);
    
    // expression.cpp
    void run_expression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> type);
    void run_iexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    void run_fexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    void run_sexpression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr);
    
protected:
    std::shared_ptr<AstTree> tree;
    std::map<std::string, std::shared_ptr<AstFunction>> function_map;
};

