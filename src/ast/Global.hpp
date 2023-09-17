//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <ast/Types.hpp>

class AstStatement;

// Represents a function, external declaration, or global variable
class AstGlobalStatement : public AstNode {
public:
    explicit AstGlobalStatement() : AstNode(V_AstType::None) {}
    explicit AstGlobalStatement(V_AstType type) : AstNode(type) {}
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
};

// Represents an extern function
class AstExternFunction : public AstGlobalStatement {
public:
    explicit AstExternFunction(std::string name) : AstGlobalStatement(V_AstType::ExternFunc) {
        this->name = name;
    }
    
    void addArgument(Var arg) { this->args.push_back(arg); }
    void setArguments(std::vector<Var> args) { this->args = args; }
    
    void setDataType(AstDataType *dataType) {
        this->dataType = dataType;
    }
    
    void setVarArgs() { this->varargs = true; }
    bool isVarArgs() { return this->varargs; }
    
    std::string getName() { return name; }
    AstDataType *getDataType() { return dataType; }
    std::vector<Var> getArguments() { return args; }
    
    void print() override;
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::vector<Var> args;
    AstDataType *dataType;
    bool varargs = false;
};

// Represents a function
class AstFunction : public AstGlobalStatement {
public:
    explicit AstFunction(std::string name) : AstGlobalStatement(V_AstType::Func) {
        this->name = name;
        block = new AstBlock;
    }
    
    std::string getName() { return name; }
    AstDataType *getDataType() { return dataType; }
    std::vector<Var> getArguments() { return args; }
    AstBlock *getBlock() { return block; }
    
    void setName(std::string name) { this->name = name; }
    
    void setArguments(std::vector<Var> args) { this->args = args; }
    
    void addStatement(std::shared_ptr<AstStatement> statement) {
        block->addStatement(statement);
    }
    
    void setDataType(AstDataType *dataType) {
        this->dataType = dataType;
    }
    
    void print() override;
    std::string dot(std::string parent) override;
private:
    std::string name = "";
    std::vector<Var> args;
    AstBlock *block;
    AstDataType *dataType;
    std::string dtName = "";
};
