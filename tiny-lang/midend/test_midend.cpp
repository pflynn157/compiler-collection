#include <iostream>

#include "test_midend.hpp"

void TestMidend::process_block(std::shared_ptr<AstBlock> block) {
    std::cout << "BLOCK" << std::endl;
}

void TestMidend::process_statement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "STATEMENT" << std::endl;
}

void TestMidend::process_extern_function(std::shared_ptr<AstExternFunction> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "EXTERN" << std::endl;
}

void TestMidend::process_function(std::shared_ptr<AstFunction> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "FUNC" << std::endl;
}

void TestMidend::process_expr_statement(std::shared_ptr<AstExprStatement> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "EXPR_STMT" << std::endl;
}

void TestMidend::process_function_call(std::shared_ptr<AstFuncCallStmt> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "FUNC_CALL" << std::endl;
}

void TestMidend::process_return(std::shared_ptr<AstReturnStmt> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "RETURN" << std::endl;
}

void TestMidend::process_var_decl(std::shared_ptr<AstVarDec> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "VAR_DECL" << std::endl;
}

void TestMidend::process_struct_decl(std::shared_ptr<AstStructDec> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "STRUCT_DECL" << std::endl;
}

void TestMidend::process_conditional(std::shared_ptr<AstIfStmt> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "COND" << std::endl;
}

void TestMidend::process_while(std::shared_ptr<AstWhileStmt> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "WHILE" << std::endl;
}

void TestMidend::process_break(std::shared_ptr<AstBreak> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "BREAK" << std::endl;
}

void TestMidend::process_continue(std::shared_ptr<AstContinue> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "  ";
    std::cout << "  ";
    std::cout << "CONTINUE" << std::endl;
}

void TestMidend::process_expression(std::shared_ptr<AstExpression> expr) {
    std::cout << "  "; std::cout << "  "; std::cout << "  ";
    std::cout << "EXPRESSION" << std::endl;
}

