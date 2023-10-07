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
    
    block->print();
}

//
// Data Types
//
void AstDataType::print() {
    if (is_unsigned) std::cout << "unsigned ";

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
    base_type->print();
}

void AstStructType::print() {
    std::cout << "struct(" << name << ")";
}

void AstObjectType::print() {
    std::cout << "object(" << name << ")";
}

//
// Global types
//
void AstExternFunction::print() {
    std::cout << "EXTERN FUNC " << name << "(";
    for (auto var : args) {
        if (var.type) var.type->print();
        std::cout << ", ";
    }
    std::cout << ") ";
    std::cout << " -> ";
    data_type->print();
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
    data_type->print();
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
        switch (stmt->type) {
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
        if (default_expressions[var.name])
            default_expressions[var.name]->print();
        else
            std::cout << "NULL";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void AstClass::print() {
    std::cout << "CLASS " << name << std::endl;
    
    for (auto const &stmt : functions) {
        std::cout << "  ";
        stmt->print();
    }
    std::cout << std::endl;
}

void AstExprStatement::print() {
    std::cout << "EXPR ";
    if (name != "") std::cout << "N:" << name << " ";
    if (dataType) dataType->print();
    
    std::cout << " ";
    expression->print();
    std::cout << std::endl;
}

void AstFuncCallStmt::print() {
    std::cout << "FC ";
    if (object_name != "") {
        std::cout << object_name << ".";
    }
    std::cout << name;
    if (expression) expression->print();
    else std::cout << "()";
    std::cout << std::endl;
}

void AstReturnStmt::print() {
    std::cout << "RETURN ";
    if (expression) expression->print();
    std::cout << std::endl;
}

void AstVarDec::print() {
    std::cout << "VAR_DEC " << name << " : ";
    data_type->print();
    if (data_type->type == V_AstType::Ptr) {
        std::cout << "[";
        size->print();
        std::cout << "]";
    }
    std::cout << std::endl;
}

void AstStructDec::print() {
    std::cout << "STRUCT " << var_name << " : " << struct_name;
    if (no_init) std::cout << " NOINIT";
    std::cout << std::endl;
}

void AstIfStmt::print(int indent) {
    std::cout << "IF ";
    expression->print();
    std::cout << " THEN" << std::endl;
    true_block->print(indent+4);
    false_block->print(indent+4);
}

void AstWhileStmt::print(int indent) {
    std::cout << "WHILE ";
    expression->print();
    std::cout << " DO" << std::endl;
    
    block->print(indent+4);
}

void AstRepeatStmt::print(int indent) {
    std::cout << "    ";
    std::cout << "REPEAT" << std::endl;
    block->print(indent+4);
}

void AstForStmt::print(int indent) {
    std::cout << "    ";
    std::cout << "FOR ";
    indexVar->print();
    std::cout << " IN ";
    startBound->print();
    std::cout << " .. ";
    endBound->print();
    std::cout << " STEP ";
    step->print();
    std::cout << std::endl;
    
    block->print(indent+4);
}

void AstForAllStmt::print(int indent) {
    std::cout << "    ";
    std::cout << "FORALL ";
    indexVar->print();
    std::cout << " IN ";
    arrayVar->print();
    std::cout << std::endl;
    
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
    value->print();
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

void AstLshOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") << (";
    rval->print();
    std::cout << ")";
}

void AstRshOp::print() {
    std::cout << "(";
    lval->print();
    std::cout << ") >> (";
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
    std::cout << "CHAR(" << value << ")";
}

void AstI8::print() {
    std::cout << value;
}

void AstI16::print() {
    std::cout << value;
}

void AstI32::print() {
    std::cout << value;
}

void AstI64::print() {
    std::cout << value;
}

void AstFloat::print() {
    std::cout << value;
}

void AstString::print() {
    std::cout << "\"" << value << "\"";
}

void AstID::print() {
    std::cout << value;
}

void AstArrayAccess::print() {
    std::cout << value << "[";
    index->print();
    std::cout << "]";
}

void AstStructAccess::print() {
    std::cout << var << "." << member;
    if (access_expression) {
        std::cout << "[";
        access_expression->print();
        std::cout << "]";
    }
}

void AstFuncCallExpr::print() {
    std::cout << name << "(";
    args->print();
    std::cout << ")";
}

void AstSizeof::print() {
    std::cout << "SIZEOF(";
    value->print();
    std::cout << ")";
}

