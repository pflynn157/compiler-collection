//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>

#include <ast/Types.hpp>
#include <ast/Global.hpp>
#include <ast/Statement.hpp>
#include <ast/Expression.hpp>

// Forward declarations
//class AstGlobalStatement;
class AstStatement;
class AstExpression;
class AstStruct;
class AstClass;

// Represents an AST tree
class AstTree {
public:
    explicit AstTree(std::string file) { this-> file = file; }
    ~AstTree() {}
    
    std::vector<AstGlobalStatement *> getGlobalStatements() {
        return global_statements;
    }
    
    std::vector<AstStruct *> getStructs() {
        return structs;
    }
    
    std::vector<AstClass *> getClasses() {
        return classes;
    }
    
    void addGlobalStatement(AstGlobalStatement *stmt) {
        global_statements.push_back(stmt);
    }
    
    void addStruct(AstStruct *s) {
        structs.push_back(s);
    }
    
    void addClass(AstClass *c) {
        classes.push_back(c);
    }
    
    void print();
private:
    std::string file = "";
    std::vector<AstGlobalStatement *> global_statements;
    std::vector<AstStruct *> structs;
    std::vector<AstClass *> classes;
};
