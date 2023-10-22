//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <map>

#include "lex.hpp"

class Pass1 {
public:
    explicit Pass1(std::string input);
    std::map<std::string, int> run();
private:
    Lex *lex;
};

