//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
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
    void process_function_call(std::shared_ptr<AstFuncCallStmt> call, std::shared_ptr<AstBlock> block) override;
    std::shared_ptr<AstExpression> process_binary_op(std::shared_ptr<AstBinaryOp> expr, std::shared_ptr<AstBlock> block) override;
};

