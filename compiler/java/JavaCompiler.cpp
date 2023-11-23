//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <cstdio>
#include <iostream>
#include <memory>

#include <JavaCompiler.hpp>

Compiler::Compiler(std::string className) {
    this->className = className;
    builder = std::make_shared<JavaClassBuilder>(className);
    
    builder->ImportField("java/lang/System", "java/io/PrintStream", "out");
    builder->ImportMethod("java/io/PrintStream", "println", "(Ljava/lang/String;)V");
    builder->ImportMethod("java/io/PrintStream", "println", "(I)V");
}

void Compiler::Build(std::shared_ptr<AstTree> tree) {
    // Generate the default constructor
    // TODO: We should check if there's a constructor before doing this
    std::shared_ptr<JavaFunction> construct = builder->CreateMethod("<init>", "()V");

    builder->CreateALoad(construct, 0);
    builder->CreateInvokeSpecial(construct, "<init>", "java/lang/Object");
    builder->CreateRetVoid(construct);

    // Build the functions (declarations only)
    for (auto GS : tree->block->block) {
        if (GS->type == V_AstType::Func) {
            BuildFunction(GS);
        }
    }
    
    // Now the code
    for (auto GS : tree->block->block) {
        if (GS->type == V_AstType::Func) {
            auto funcAst = std::static_pointer_cast<AstFunction>(GS);
            std::shared_ptr<JavaFunction> func = funcMap[funcAst->name];
            
            for (auto const &stmt : funcAst->block->block) {
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
void Compiler::BuildFunction(std::shared_ptr<AstStatement> GS) {
    auto func = std::static_pointer_cast<AstFunction>(GS);
    
    int flags = 0;
    if (func->routine) flags |= F_STATIC;
    
    switch (func->attr) {
        case Attr::Public: flags |= F_PUBLIC; break;
        case Attr::Protected: flags |= F_PROTECTED; break;
        case Attr::Private: flags |= F_PRIVATE; break;
    }
    
    std::string signature = "()V";
    if (func->name == "main") signature = "([Ljava/lang/String;)V";
    
    std::shared_ptr<JavaFunction> function = builder->CreateMethod(func->name, signature, flags);
    funcMap[func->name] = function;
}

// Builds a statement
void Compiler::BuildStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function) {
    switch (stmt->type) {
        case V_AstType::VarDec: BuildVarDec(stmt, function); break;
        case V_AstType::ExprStmt: BuildVarAssign(stmt, function); break;
    
        case V_AstType::FuncCallStmt: BuildFuncCallStatement(stmt, function); break;
    
        case V_AstType::Return: {
            if (!stmt->hasExpression()) {
                builder->CreateRetVoid(function);
            } else {
                // TODO
            }
        } break;
        
        default: {}
    }
}

// Builds a variable declaration
void Compiler::BuildVarDec(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function) {
    auto vd = std::static_pointer_cast<AstVarDec>(stmt);
    
    switch (vd->data_type->type) {
        case V_AstType::Int32: {
            intMap[vd->name] = iCount;
            ++iCount;
        } break;
    
        case V_AstType::Object: {
            objMap[vd->name] = aCount;
            ++aCount;
            
            objTypeMap[vd->name] = vd->class_name;
            
            builder->CreateNew(function, vd->class_name);
            builder->CreateDup(function);
            builder->CreateInvokeSpecial(function, "<init>", vd->class_name);
            builder->CreateAStore(function, aCount - 1);
        } break;
        
        default: {}
    }
}

// Builds a variable assignment
void Compiler::BuildVarAssign(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function) {
    auto va = std::static_pointer_cast<AstExprStatement>(stmt);
    BuildExpr(va->expression, function, va->dataType);
    
    // TODO: Eventually get rid of this when we migrate from Espresso
    if (va->expression->type != V_AstType::Assign) {
        switch (va->dataType->type) {
            case V_AstType::Int32: {
                int iPos = intMap[va->name];
                builder->CreateIStore(function, iPos);
            } break;
            
            default: {}
        }
        return;
    }
}

// Builds a function call statement
void Compiler::BuildFuncCallStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function) {
    auto fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
    
    if (fc->name == "println" || fc->name == "print") {
        builder->CreateGetStatic(function, "out");
    }
    
    std::string signature = "";
    std::string baseClass = "";
    
    if (fc->object_name == "this") {
        baseClass = "this";
        builder->CreateALoad(function, 0);
    } else if (fc->object_name != "") {
        baseClass = objTypeMap[fc->object_name];
        //if (baseClass == className) baseClass = "";
        
        int pos = objMap[fc->object_name];
        builder->CreateALoad(function, pos);
    }
    
    /*auto list = std::static_pointer_cast<AstExprList>(fc->expression);
    for (auto const &expr : list->list) {
        signature = GetTypeForExpr(expr);
        BuildExpr(expr, function);
    }*/
    signature = GetTypeForExpr(fc->expression);
    BuildExpr(fc->expression, function);
    
    signature = "(" + signature + ")V";
    //fc->expression->print(); puts("");
    //printf("NAME: %s | Sig: %s\n", fc->name.c_str(), signature.c_str());
    builder->CreateInvokeVirtual(function, fc->name, baseClass, signature);
}

// Builds an expression
void Compiler::BuildExpr(std::shared_ptr<AstExpression> expr, std::shared_ptr<JavaFunction> function, std::shared_ptr<AstDataType> dataType) {
    if (expr == nullptr) return;

    switch (expr->type) {
        case V_AstType::ExprList: {
            auto list = std::static_pointer_cast<AstExprList>(expr);
            for (auto const &item : list->list) {
                BuildExpr(item, function, dataType);
            }
        } break;
    
        case V_AstType::IntL: {
            auto i = std::static_pointer_cast<AstInt>(expr);
            builder->CreateBIPush(function, i->value);
        } break;
    
        case V_AstType::StringL: {
            std::shared_ptr<AstString> str = std::static_pointer_cast<AstString>(expr);
            builder->CreateString(function, str->value);
        } break;
        
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(expr);
            int pos = intMap[id->value];
            builder->CreateILoad(function, pos);
            /*switch (dataType->type) {
                case V_AstType::Int32: {
                    int pos = intMap[id->value];
                    builder->CreateILoad(function, pos);
                } break;
                
                default: {
                    if (intMap.find(id->value) != intMap.end()) {
                        int pos = intMap[id->value];
                        builder->CreateILoad(function, pos);
                    }
                }
            }*/
        } break;
        
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            BuildExpr(op->rval, function, dataType);
            
            auto id = std::static_pointer_cast<AstID>(op->lval);
            switch (dataType->type) {
                case V_AstType::Int32: {
                    int iPos = intMap[id->value];
                    builder->CreateIStore(function, iPos);
                } break;
                
                default: {}
            }
        } break;
        
        case V_AstType::Add: 
        case V_AstType::Sub:
        case V_AstType::Mul:
        case V_AstType::Div:
        case V_AstType::Mod:
        case V_AstType::And:
        case V_AstType::Or:
        case V_AstType::Xor:
        case V_AstType::Lsh:
        case V_AstType::Rsh:
        {
            std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(expr);
            BuildExpr(op->lval, function, dataType);
            BuildExpr(op->rval, function, dataType);
            
            // Math
            if (expr->type == V_AstType::Add)
                builder->CreateIAdd(function);
            else if (expr->type == V_AstType::Sub)
                builder->CreateISub(function);
            else if (expr->type == V_AstType::Mul)
                builder->CreateIMul(function);
            else if (expr->type == V_AstType::Div)
                builder->CreateIDiv(function);
            else if (expr->type == V_AstType::Mod)
                builder->CreateIRem(function);
            
            // Logical
            else if (expr->type == V_AstType::And)
                builder->CreateIAnd(function);
            else if (expr->type == V_AstType::Or)
                builder->CreateIOr(function);
            else if (expr->type == V_AstType::Xor)
                builder->CreateIXor(function);
            else if (expr->type == V_AstType::Lsh)
                builder->CreateIShl(function);
            else if (expr->type == V_AstType::Rsh)
                builder->CreateIShr(function);
        } break;
        
        default: {}
    }
}

// Returns a type value for an expression
std::string Compiler::GetTypeForExpr(std::shared_ptr<AstExpression> expr) {
    if (expr == nullptr) return "";

    switch (expr->type) {
        case V_AstType::IntL: return "I";
        case V_AstType::StringL: return "Ljava/lang/String;";
        
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(expr);
            
            if (intMap.find(id->value) != intMap.end()) {
                return "I";
            }
        } break;
        
        case V_AstType::ExprList: {
            auto list = std::static_pointer_cast<AstExprList>(expr);
            return GetTypeForExpr(list->list[0]);
        };
        
        default: {}
    }

    return "V";
}
