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
private:
    std::unique_ptr<Lex> lex;
};

