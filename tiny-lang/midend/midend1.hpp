//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <memory>

#include <ast/ast.hpp>

//
// The main class for calling and managing the midend passes
//
struct Midend1 {
    explicit Midend1(std::shared_ptr<AstTree> tree);
    void run();
    
    // Member variables
    std::shared_ptr<AstTree> tree;
};

