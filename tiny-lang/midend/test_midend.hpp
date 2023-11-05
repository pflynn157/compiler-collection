#pragma once

#include <midend/ast_midend.hpp>
#include <ast/ast.hpp>

class TestMidend : public AstMidend {
public:
    explicit TestMidend(std::shared_ptr<AstTree> tree) : AstMidend(tree) {}
    
    void process_block(std::shared_ptr<AstBlock> block) override;
    
    void process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block) override;
    void process_extern_function(std::shared_ptr<AstExternFunction> stmt, std::shared_ptr<AstBlock> block) override;
    void process_function(std::shared_ptr<AstFunction> stmt, std::shared_ptr<AstBlock> block) override;
    void process_expr_statement(std::shared_ptr<AstExprStatement> stmt, std::shared_ptr<AstBlock> block) override;
    void process_function_call(std::shared_ptr<AstFuncCallStmt> stmt, std::shared_ptr<AstBlock> block) override;
    void process_return(std::shared_ptr<AstReturnStmt> stmt, std::shared_ptr<AstBlock> block) override;
    void process_var_decl(std::shared_ptr<AstVarDec> stmt, std::shared_ptr<AstBlock> block) override;
    void process_struct_decl(std::shared_ptr<AstStructDec> stmt, std::shared_ptr<AstBlock> block) override;
    void process_conditional(std::shared_ptr<AstIfStmt> stmt, std::shared_ptr<AstBlock> block) override;
    void process_while(std::shared_ptr<AstWhileStmt> stmt, std::shared_ptr<AstBlock> block) override;
    void process_break(std::shared_ptr<AstBreak> stmt, std::shared_ptr<AstBlock> block) override;
    void process_continue(std::shared_ptr<AstContinue> stmt, std::shared_ptr<AstBlock> block) override;
    
    void process_expression(std::shared_ptr<AstExpression> expr) override;
};

