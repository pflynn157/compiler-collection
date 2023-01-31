#pragma once

#include <string>

#include <ast/ast.hpp>

#include "ptasm.hpp"

class Compiler {
public:
    explicit Compiler(std::string name);
    void compile(AstTree *tree);
    AsmFile *getFile() { return file; }
protected:
    void compileFunction(AstGlobalStatement *global);
    void compileBlock(AsmFunction *func, AstBlock *block);
    void compileStatement(AsmFunction *func, AsmBlock *block, AstStatement *stmt);
    AsmOperand *compileExpression(AsmBlock *block, AstExpression *expr);
private:
    AsmFile *file;
    int label_counter = 0;
};

