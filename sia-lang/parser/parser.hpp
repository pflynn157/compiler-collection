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
    bool is_constant(int tk) override;
    bool is_id(int tk) override;
    bool is_list_delim(int tk) override;
    std::shared_ptr<AstExpression> build_constant(int tk) override;
    bool build_identifier(std::shared_ptr<AstBlock> block, int tk, std::shared_ptr<ExprContext> ctx) override;
    
    // variable.cpp
    void parse_scalar(std::shared_ptr<AstBlock> block);
    
private:
    std::string module_name = "";
};

