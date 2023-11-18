#pragma once

#include <memory>

#include <ast/ast.hpp>

//
// The main class for calling and managing the midend passes
//
class ParallelMidend {
public:
    explicit ParallelMidend(std::shared_ptr<AstTree> tree);
    void run();
    
    std::shared_ptr<AstTree> tree;
private:
    std::shared_ptr<AstTree> parse_tree;
    int index = 0;
    
    void it_process_block(std::shared_ptr<AstBlock> &block, std::shared_ptr<AstBlock> &new_block);
    void process_block_statement(std::shared_ptr<AstBlockStmt> &stmt, std::shared_ptr<AstBlock> &block);
    void build_omp_parallel_for(std::shared_ptr<AstFunction> func, std::shared_ptr<AstStatement> first);
};

