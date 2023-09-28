#pragma once

#include <string>
#include <memory>

#include <parser/base_parser.hpp>
#include <lex/lex.hpp>

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    bool parse();
    
    bool parse_block(std::shared_ptr<AstBlock> block);
    
    std::unique_ptr<Lex> lex;
};

