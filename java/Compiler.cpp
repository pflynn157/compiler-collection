//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <cstdio>
#include <iostream>

#include <Compiler.hpp>

Compiler::Compiler(std::string className) {
    this->className = className;
    builder = new JavaClassBuilder(className);
    
    builder->ImportField("java/lang/System", "java/io/PrintStream", "out");
    builder->ImportMethod("java/io/PrintStream", "println", "(Ljava/lang/String;)V");
    builder->ImportMethod("java/io/PrintStream", "println", "(I)V");
}

void Compiler::Build(AstTree *tree) {
    // Generate the default constructor
    // TODO: We should check if there's a constructor before doing this
    JavaFunction *construct = builder->CreateMethod("<init>", "()V");

    builder->CreateALoad(construct, 0);
    builder->CreateInvokeSpecial(construct, "<init>", "java/lang/Object");
    builder->CreateRetVoid(construct);

    // Build the functions (declarations only)
    for (auto GS : tree->getGlobalStatements()) {
        if (GS->getType() == AstType::Func) {
            BuildFunction(GS);
        }
    }
    
    // Now the code
    for (auto GS : tree->getGlobalStatements()) {
        if (GS->getType() == AstType::Func) {
            AstFunction *funcAst = static_cast<AstFunction *>(GS);
            JavaFunction *func = funcMap[funcAst->getName()];
            
            for (AstStatement *stmt : funcAst->getBlock()->getBlock()) {
                BuildStatement(stmt, func);
            }
        }
    }
}

void Compiler::Write() {
    std::string fullName = className + ".class";
    
    FILE *file = fopen(fullName.c_str(), "wb");
    builder->Write(file);
    fclose(file);
}

// Builds a function
void Compiler::BuildFunction(AstGlobalStatement *GS) {
    AstFunction *func = static_cast<AstFunction *>(GS);
    
    int flags = 0;
    if (func->isRoutine()) flags |= F_STATIC;
    
    switch (func->getAttribute()) {
        case Attr::Public: flags |= F_PUBLIC; break;
        case Attr::Protected: flags |= F_PROTECTED; break;
        case Attr::Private: flags |= F_PRIVATE; break;
    }
    
    std::string signature = "()V";
    if (func->getName() == "main") signature = "([Ljava/lang/String;)V";
    
    JavaFunction *function = builder->CreateMethod(func->getName(), signature, flags);
    funcMap[func->getName()] = function;
    
    /*for (AstStatement *stmt : func->getBlock()->getBlock()) {
        BuildStatement(stmt, function);
    }*/
}

// Builds a statement
void Compiler::BuildStatement(AstStatement *stmt, JavaFunction *function) {
    switch (stmt->getType()) {
        case AstType::VarDec: BuildVarDec(stmt, function); break;
        case AstType::VarAssign: BuildVarAssign(stmt, function); break;
    
        case AstType::FuncCallStmt: BuildFuncCallStatement(stmt, function); break;
    
        case AstType::Return: {
            if (stmt->getExpressionCount() == 0) {
                builder->CreateRetVoid(function);
            } else {
                // TODO
            }
        } break;
        
        default: {}
    }
}

// Builds a variable declaration
void Compiler::BuildVarDec(AstStatement *stmt, JavaFunction *function) {
    AstVarDec *vd = static_cast<AstVarDec *>(stmt);
    
    switch (vd->getDataType()) {
        case DataType::Int32: {
            intMap[vd->getName()] = iCount;
            ++iCount;
        } break;
    
        case DataType::Object: {
            objMap[vd->getName()] = aCount;
            ++aCount;
            
            objTypeMap[vd->getName()] = vd->getClassName();
            
            builder->CreateNew(function, vd->getClassName());
            builder->CreateDup(function);
            builder->CreateInvokeSpecial(function, "<init>", vd->getClassName());
            builder->CreateAStore(function, aCount - 1);
        } break;
        
        default: {}
    }
}

// Builds a variable assignment
void Compiler::BuildVarAssign(AstStatement *stmt, JavaFunction *function) {
    AstVarAssign *va = static_cast<AstVarAssign *>(stmt);
    
    BuildExpr(va->getExpression(), function, va->getDataType());
    
    switch (va->getDataType()) {
        case DataType::Int32: {
            int iPos = intMap[va->getName()];
            builder->CreateIStore(function, iPos);
        } break;
        
        default: {}
    }
}

// Builds a function call statement
void Compiler::BuildFuncCallStatement(AstStatement *stmt, JavaFunction *function) {
    AstFuncCallStmt *fc = static_cast<AstFuncCallStmt *>(stmt);
    
    if (fc->getName() == "println") {
        builder->CreateGetStatic(function, "out");
    }
    
    std::string signature = "";
    std::string baseClass = "";
    
    if (fc->getObjectName() == "this") {
        baseClass = "this";
        builder->CreateALoad(function, 0);
    } else if (fc->getObjectName() != "") {
        baseClass = objTypeMap[fc->getObjectName()];
        //if (baseClass == className) baseClass = "";
        
        int pos = objMap[fc->getObjectName()];
        builder->CreateALoad(function, pos);
    }
    
    for (AstExpression *expr : fc->getExpressions()) {
        signature = GetTypeForExpr(expr);
        BuildExpr(expr, function);
    }
    
    signature = "(" + signature + ")V";
    builder->CreateInvokeVirtual(function, fc->getName(), baseClass, signature);
}

// Builds an expression
void Compiler::BuildExpr(AstExpression *expr, JavaFunction *function, DataType dataType) {
    switch (expr->getType()) {
        case AstType::IntL: {
            AstInt *i = static_cast<AstInt *>(expr);
            builder->CreateBIPush(function, i->getValue());
        } break;
    
        case AstType::StringL: {
            AstString *str = static_cast<AstString *>(expr);
            builder->CreateString(function, str->getValue());
        } break;
        
        case AstType::ID: {
            AstID *id = static_cast<AstID *>(expr);
            switch (dataType) {
                case DataType::Int32: {
                    int pos = intMap[id->getValue()];
                    builder->CreateILoad(function, pos);
                } break;
                
                default: {
                    if (intMap.find(id->getValue()) != intMap.end()) {
                        int pos = intMap[id->getValue()];
                        builder->CreateILoad(function, pos);
                    }
                }
            }
        } break;
        
        case AstType::Add: 
        case AstType::Sub:
        case AstType::Mul:
        case AstType::Div:
        case AstType::Rem:
        case AstType::And:
        case AstType::Or:
        case AstType::Xor:
        case AstType::Lsh:
        case AstType::Rsh: {
            AstBinaryOp *op = static_cast<AstBinaryOp *>(expr);
            BuildExpr(op->getLVal(), function, dataType);
            BuildExpr(op->getRVal(), function, dataType);
            
            // Math
            if (expr->getType() == AstType::Add)
                builder->CreateIAdd(function);
            else if (expr->getType() == AstType::Sub)
                builder->CreateISub(function);
            else if (expr->getType() == AstType::Mul)
                builder->CreateIMul(function);
            else if (expr->getType() == AstType::Div)
                builder->CreateIDiv(function);
            else if (expr->getType() == AstType::Rem)
                builder->CreateIRem(function);
            
            // Logical
            else if (expr->getType() == AstType::And)
                builder->CreateIAnd(function);
            else if (expr->getType() == AstType::Or)
                builder->CreateIOr(function);
            else if (expr->getType() == AstType::Xor)
                builder->CreateIXor(function);
            else if (expr->getType() == AstType::Lsh)
                builder->CreateIShl(function);
            else if (expr->getType() == AstType::Rsh)
                builder->CreateIShr(function);
        } break;
        
        default: {}
    }
}

// Returns a type value for an expression
std::string Compiler::GetTypeForExpr(AstExpression *expr) {
    switch (expr->getType()) {
        case AstType::IntL: return "I";
        case AstType::StringL: return "Ljava/lang/String;";
        
        case AstType::ID: {
            AstID *id = static_cast<AstID *>(expr);
            
            if (intMap.find(id->getValue()) != intMap.end()) {
                return "I";
            }
        } break;
        
        default: {}
    }

    return "V";
}
