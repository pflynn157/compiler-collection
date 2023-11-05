#pragma once

#include <memory>

#include <ast/ast.hpp>

class AstMidend {
public:
    explicit AstMidend(std::shared_ptr<AstTree> tree);
    void run();
    
    //
    // Public-facing processing statements
    //
    // Blocks
    virtual void process_block(std::shared_ptr<AstBlock> block) {}
    
    // Statements
    virtual void process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_extern_function(std::shared_ptr<AstExternFunction> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_function(std::shared_ptr<AstFunction> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_expr_statement(std::shared_ptr<AstExprStatement> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_function_call(std::shared_ptr<AstFuncCallStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_return(std::shared_ptr<AstReturnStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_var_decl(std::shared_ptr<AstVarDec> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_struct_decl(std::shared_ptr<AstStructDec> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_conditional(std::shared_ptr<AstIfStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_while(std::shared_ptr<AstWhileStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_repeat(std::shared_ptr<AstRepeatStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_for(std::shared_ptr<AstForStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_forall(std::shared_ptr<AstForAllStmt> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_break(std::shared_ptr<AstBreak> stmt, std::shared_ptr<AstBlock> block) {}
    virtual void process_continue(std::shared_ptr<AstContinue> stmt, std::shared_ptr<AstBlock> block) {}
    
    // Expressions
    virtual void process_expression(std::shared_ptr<AstExpression> expr) {}
    virtual void process_expression_list(std::shared_ptr<AstExprList> expr) {}
    virtual void process_op(std::shared_ptr<AstOp> expr) {}
    virtual void process_unary_op(std::shared_ptr<AstUnaryOp> expr) {}
    virtual void process_neg_op(std::shared_ptr<AstNegOp> expr) {}
    virtual void process_binary_op(std::shared_ptr<AstBinaryOp> expr) {}
    virtual void process_assign_op(std::shared_ptr<AstAssignOp> expr) {}
    virtual void process_add_op(std::shared_ptr<AstAddOp> expr) {}
    virtual void process_sub_op(std::shared_ptr<AstSubOp> expr) {}
    virtual void process_mul_op(std::shared_ptr<AstMulOp> expr) {}
    virtual void process_div_op(std::shared_ptr<AstDivOp> expr) {}
    virtual void process_mod_op(std::shared_ptr<AstModOp> expr) {}
    virtual void process_and_op(std::shared_ptr<AstAndOp> expr) {}
    virtual void process_or_op(std::shared_ptr<AstOrOp> expr) {}
    virtual void process_xor_op(std::shared_ptr<AstXorOp> expr) {}
    virtual void process_lsh_op(std::shared_ptr<AstLshOp> expr) {}
    virtual void process_rsh_op(std::shared_ptr<AstRshOp> expr) {}
    virtual void process_eq_op(std::shared_ptr<AstEQOp> expr) {}
    virtual void process_neq_op(std::shared_ptr<AstNEQOp> expr) {}
    virtual void process_gt_op(std::shared_ptr<AstGTOp> expr) {}
    virtual void process_lt_op(std::shared_ptr<AstLTOp> expr) {}
    virtual void process_gte_op(std::shared_ptr<AstGTEOp> expr) {}
    virtual void process_lte_op(std::shared_ptr<AstLTEOp> expr) {}
    virtual void process_logical_and_op(std::shared_ptr<AstLogicalAndOp> expr) {}
    virtual void process_logical_or_op(std::shared_ptr<AstLogicalOrOp> expr) {}
    // TODO: Finish
private:
    std::shared_ptr<AstTree> tree;
    
    // Functions
    void it_process_block(std::shared_ptr<AstBlock> block);
    void it_process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block);
    void it_process_expression(std::shared_ptr<AstExpression> expr);
};

