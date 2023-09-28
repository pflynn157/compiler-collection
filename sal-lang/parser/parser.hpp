#pragma once

#include <string>

#include <parser/base_parser.hpp>

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    bool parse();
};

