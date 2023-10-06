#pragma once

#include <string>

#include <ast.hpp>
#include <LLIR/LLIR.hpp>
#include <LLIR/LLIRBuilder.hpp>
#include <LLIR/PASM.hpp>

class LLIRCompiler {
public:
    explicit LLIRCompiler(AstTree *tree, std::string name);
    void compile();
    void debug();
protected:
    void compileLLIR();
    void compileFunction(AstGlobalStatement *global);
    void compileStatement(AstStatement *stmt);
    LLIRValue *compileValue(AstExpression *expr);
    LLIRType *translateType(DataType dataType, DataType subType);
    void compilePASM();
private:
    AstTree *tree;
    std::string name = "";
    
    LLIRFile *llir;
    LLIRBuilder *builder;
    PASMFile *pasm;
};

