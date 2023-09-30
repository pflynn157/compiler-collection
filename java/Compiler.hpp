//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <map>
#include <memory>

#include <ast/ast.hpp>

#include <Java/JavaBuilder.hpp>

std::string GetClassName(std::string input);

class Compiler {
public:
    explicit Compiler(std::string className);
    void Build(std::shared_ptr<AstTree> tree);
    void Write();
protected:
    void BuildFunction(std::shared_ptr<AstStatement> GS);
    void BuildStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    
    void BuildVarDec(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildVarAssign(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildFuncCallStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildExpr(AstExpression *expr, JavaFunction *function, DataType dataType = DataType::Void);
    
    std::string GetTypeForExpr(AstExpression *expr);
private:
    std::string className;
    JavaClassBuilder *builder;
    std::map<std::string, JavaFunction *> funcMap;
    
    int aCount = 1;
    std::map<std::string, int> objMap;
    std::map<std::string, std::string> objTypeMap;
    
    int iCount = 1;
    std::map<std::string, int> intMap;
};
