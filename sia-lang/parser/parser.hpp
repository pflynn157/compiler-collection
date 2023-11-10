#pragma once

#include <string>
#include <memory>

#include <parser/base_parser.hpp>
#include <ast/ast.hpp>

#include <lex/lex.hpp>

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    bool parse() override;
    
    // parser.cpp
    void parse_block(std::shared_ptr<AstBlock> block);
    void consume_token(token t, std::string message);
    std::string generate_name(std::string base);
    std::shared_ptr<AstDataType> get_data_type();
    
    // flow.cpp
    void parse_iter(std::shared_ptr<AstBlock> block);
    
    // function.cpp
    void parse_arguments(std::vector<Var> &args);
    void parse_function();
    void parse_return(std::shared_ptr<AstBlock> block);
    void parse_function_call(std::shared_ptr<AstBlock> block, std::string name);
    
    // expression.cpp
    std::shared_ptr<AstExpression> parse_expression(std::shared_ptr<AstBlock> block, token stop = t_period, bool is_list = false);
    
    // variable.cpp
    void parse_scalar(std::shared_ptr<AstBlock> block);
    
private:
    std::unique_ptr<Lex> lex;
    std::string module_name = "";
};

