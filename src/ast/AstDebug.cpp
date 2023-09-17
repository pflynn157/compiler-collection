//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <ast/ast.hpp>

void AstTree::print() {
    std::cout << "FILE: " << file << std::endl;
    std::cout << std::endl;
    
    for (auto str : structs) str->print();
    
    for (auto stmt : global_statements) {
        stmt->print();
    }
}

//
// Data Types
//
void AstDataType::print() {
    if (_isUnsigned) std::cout << "unsigned ";

    switch (type) {
        case V_AstType::Void: std::cout << "void"; break;
        case V_AstType::Bool: std::cout << "bool"; break;
        case V_AstType::Char: std::cout << "char"; break;
        case V_AstType::Int8: std::cout << "int8"; break;
        case V_AstType::Int16: std::cout << "int16"; break;
        case V_AstType::Int32: std::cout << "int32"; break;
        case V_AstType::Int64: std::cout << "int64"; break;
        case V_AstType::String: std::cout << "string"; break;
        default: {}
    }
}

void AstPointerType::print() {
    std::cout << "*";
    baseType->print();
}

void AstStructType::print() {
    std::cout << "struct(" << name << ")";
}

//
// Global types
//
void AstExternFunction::print() {
    std::cout << "EXTERN FUNC " << name << "(";
    for (auto var : args) {
        var.type->print();
        std::cout << ", ";
    }
    std::cout << ") ";
    std::cout << " -> ";
    dataType->print();
    std::cout << std::endl;
}

void AstFunction::print() {
    std::cout << std::endl;
    std::cout << "FUNC " << name << "(";
    for (auto var : args) {
        std::cout << var.name << ":";
        var.type->print();
        std::cout << ", ";
    }
    std::cout << ") -> ";
    dataType->print();
    std::cout << std::endl;
    
    block->print();
}

void AstBlock::print(int indent) {
    // Print the symbol table
    for (int i = 0; i<indent; i++) std::cout << " ";
    std::cout << "[" << std::endl;
    
    for (auto const &x : symbolTable) {
        for (int i = 0; i<indent+2; i++) std::cout << " ";
        std::cout << "SYM: " << x.first << " : ";
        if (x.second) x.second->print();
        else std::cout << "<NULL TYPE>";
        std::cout << std::endl;
    }
    
    for (int i = 0; i<indent; i++) std::cout << " ";
    std::cout << "]" << std::endl;

    // Print the block
    for (int i = 0; i<indent; i++) std::cout << " ";
    std::cout << "{" << std::endl;
    
    for (auto stmt : block) {
        for (int i = 0; i<indent; i++) std::cout << " ";
        switch (stmt->getType()) {
            case V_AstType::If: {
                std::static_pointer_cast<AstIfStmt>(stmt)->print(indent);
            } break;
            case V_AstType::While: {
                std::static_pointer_cast<AstWhileStmt>(stmt)->print(indent);
            } break;
            
            default: stmt->print();
        }
    }
    
    for (int i = 0; i<indent; i++) std::cout << " ";
    std::cout << "}" << std::endl;
}

void AstStruct::print() {
    std::cout << "STRUCT " << name << std::endl;
    
    for (auto var : items) {
        std::cout << var.name << " : ";
        var.type->print();
        std::cout << " ";
        defaultExpressions[var.name]->print();
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void AstExprStatement::print() {
    std::cout << "EXPR ";
    if (dataType) dataType->print();
    
    std::cout << " ";
    getExpression()->print();
    std::cout << std::endl;
}

void AstFuncCallStmt::print() {
    std::cout << "FC " << name;
    getExpression()->print();
    std::cout << std::endl;
}

void AstReturnStmt::print() {
    std::cout << "RETURN ";
    if (getExpression()) getExpression()->print();
    std::cout << std::endl;
}

void AstVarDec::print() {
    std::cout << "VAR_DEC " << name << " : ";
    dataType->print();
    if (dataType->getType() == V_AstType::Ptr) {
        std::cout << "[";
        size->print();
        std::cout << "]";
    }
    std::cout << std::endl;
}

void AstStructDec::print() {
    std::cout << "STRUCT " << varName << " : " << structName;
    if (noInit) std::cout << " NOINIT";
    std::cout << std::endl;
}

void AstIfStmt::print(int indent) {
    std::cout << "IF ";
    getExpression()->print();
    std::cout << " THEN" << std::endl;
    trueBlock->print(indent+4);
    falseBlock->print(indent+4);
}

void AstWhileStmt::print(int indent) {
    std::cout << "WHILE ";
    getExpression()->print();
    std::cout << " DO" << std::endl;
    
    block->print(indent+4);
}

void AstBreak::print() {
    std::cout << "BREAK" << std::endl;
}

void AstContinue::print() {
    std::cout << "CONTINUE" << std::endl;
}

void AstExprList::print() {
    std::cout << "{";
    for (auto item : list) {
        item->print();
        std::cout << ", ";
    }
    std::cout << "}";
}

void AstNegOp::print() {
    std::cout << "(-";
    val->print();
    std::cout << ")";
}

void AstAssignOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") := (";
    rval->print();
    std::cout << ")";
}

void AstAddOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") + (";
    rval->print();
    std::cout << ")";
}

void AstSubOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") - (";
    rval->print();
    std::cout << ")";
}

void AstMulOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") * (";
    rval->print();
    std::cout << ")";
}

void AstDivOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") / (";
    rval->print();
    std::cout << ")";
}

void AstModOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") % (";
    rval->print();
    std::cout << ")";
}

void AstAndOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") AND (";
    rval->print();
    std::cout << ")";
}

void AstOrOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") OR (";
    rval->print();
    std::cout << ")";
}

void AstXorOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") XOR (";
    rval->print();
    std::cout << ")";
}

void AstEQOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") == (";
    rval->print();
    std::cout << ")";
}

void AstNEQOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") != (";
    rval->print();
    std::cout << ")";
}

void AstGTOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") > (";
    rval->print();
    std::cout << ")";
}

void AstLTOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") < (";
    rval->print();
    std::cout << ")";
}

void AstGTEOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") >= (";
    rval->print();
    std::cout << ")";
}

void AstLTEOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") <= (";
    rval->print();
    std::cout << ")";
}

void AstLogicalAndOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") && (";
    rval->print();
    std::cout << ")";
}

void AstLogicalOrOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") || (";
    rval->print();
    std::cout << ")";
}

void AstChar::print() {
    std::cout << "CHAR(" << val << ")";
}

void AstI8::print() {
    std::cout << val;
}

void AstI16::print() {
    std::cout << val;
}

void AstI32::print() {
    std::cout << val;
}

void AstI64::print() {
    std::cout << val;
}

void AstString::print() {
    std::cout << "\"" << val << "\"";
}

void AstID::print() {
    std::cout << val;
}

void AstArrayAccess::print() {
    std::cout << val << "[";
    index->print();
    std::cout << "]";
}

void AstStructAccess::print() {
    std::cout << var << "." << member;
}

void AstFuncCallExpr::print() {
    std::cout << name << "(";
    expr->print();
    std::cout << ")";
}

