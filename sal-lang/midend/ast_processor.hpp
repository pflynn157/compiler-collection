//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <memory>

#include <ast/ast.hpp>

//
// AstProcessor- performs default AST changes needed
// Many of these changes happen by default, but some flags can be
// specified
//
struct AstProcessor {
    explicit AstProcessor(std::shared_ptr<AstTree> tree);
    void run();
    
    // General functions for iterating over the tree
    void proc_global_statement(std::shared_ptr<AstStatement> stmt);
    void proc_struct_definition(std::shared_ptr<AstStruct> def);
    void proc_block(std::shared_ptr<AstBlock> block);
    void proc_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block);
    void proc_expression(std::shared_ptr<AstExpression> expr);
    
    // Functions for specific cases
    void proc_print(std::shared_ptr<AstFuncCallStmt> call, std::shared_ptr<AstBlock> block);
    
    // Member variables
    std::shared_ptr<AstTree> tree;
};

