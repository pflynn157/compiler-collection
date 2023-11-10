#pragma once

#include <memory>

#include <ast/ast.hpp>

//
// The main class for calling and managing the midend passes
//
class Midend {
public:
    explicit Midend(std::shared_ptr<AstTree> tree);
    void run();
    
    std::shared_ptr<AstTree> tree;
private:
    std::shared_ptr<AstTree> parse_tree;
    
    void it_process_block(std::shared_ptr<AstBlock> &block, std::shared_ptr<AstBlock> &new_block);
    void process_block_statement(std::shared_ptr<AstBlockStmt> &stmt, std::shared_ptr<AstBlock> &block);
    void process_print(std::shared_ptr<AstFuncCallStmt> &call, std::shared_ptr<AstBlock> &block);
};

