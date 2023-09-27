#pragma once

#include <memory>

#include <ast/ast.hpp>

//
// The main class for calling and managing the midend passes
//
struct Midend {
    explicit Midend(std::shared_ptr<AstTree> tree);
    void run();
    
    // Member variables
    std::shared_ptr<AstTree> tree;
};

