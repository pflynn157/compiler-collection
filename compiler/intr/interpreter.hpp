#pragma once

#include <string>
#include <memory>
#include <map>
#include <stack>

#include <ast/ast.hpp>

//
// This contains the contextual information
// Every function creates a context
//
// * var_map -> Holds variable values
// * stack -> Holds values from expression evaluation
//
struct IntrContext {
    std::map<std::string, std::shared_ptr<AstExpression>> var_map;
    std::map<std::string, std::shared_ptr<AstDataType>> type_map;
    std::stack<std::shared_ptr<AstExpression>> stack;
    
    std::shared_ptr<AstDataType> func_type;
};

//
// This handles running the actual interpreter
//
struct AstInterpreter {
    explicit AstInterpreter(std::shared_ptr<AstTree> tree);
    int run();
    
    int run_function(std::shared_ptr<AstFunction> func, std::shared_ptr<AstExprList> args);
    void run_block(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstBlock> block);
    void run_expression(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> type);
    void run_print(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExprList> args);
    
protected:
    std::shared_ptr<AstTree> tree;
    std::map<std::string, std::shared_ptr<AstFunction>> function_map;
};

