#pragma once

#include <string>
#include <memory>

#include <ast/ast.hpp>

#include "ptasm.hpp"

class Compiler {
public:
    explicit Compiler(std::string name);
    void compile(std::shared_ptr<AstTree> tree);
    AsmFile *getFile() { return file; }
protected:
    void compileFunction(std::shared_ptr<AstGlobalStatement> global);
    void compileBlock(AsmFunction *func, std::shared_ptr<AstBlock> block);
    void compileStatement(AsmFunction *func, AsmBlock *block, std::shared_ptr<AstStatement> stmt);
    AsmOperand *compileExpression(AsmBlock *block, std::shared_ptr<AstExpression> expr);
private:
    AsmFile *file;
    int label_counter = 0;
};

