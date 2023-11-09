#pragma once

#include <memory>

#include <ast/ast.hpp>
#include <midend/ast_midend.hpp>

//
// The main class for calling and managing the midend passes
//
class Midend : public AstMidend {
public:
    explicit Midend(std::shared_ptr<AstTree> tree) : AstMidend(tree) {}
    void process_block_statement(std::shared_ptr<AstBlockStmt> stmt, std::shared_ptr<AstBlock> block) override;
};

