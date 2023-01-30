//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "llvm/IR/BasicBlock.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::sys;

#include <iostream>
#include <exception>

#include "Compiler.hpp"
#include <llvm-c/Support.h>

Compiler::Compiler(AstTree *tree, CFlags cflags) {
    char const *args[] = { "", "--x86-asm-syntax=intel" };
    LLVMParseCommandLineOptions(2, args, NULL);
    
    this->tree = tree;
    this->cflags = cflags;

    context = std::make_unique<LLVMContext>();
    mod = std::make_unique<Module>(cflags.name, *context);
    builder = std::make_unique<IRBuilder<>>(*context);
}

void Compiler::compile() {
    // Build the structures used by the program
    for (auto str : tree->getStructs()) {
        std::vector<Type *> elementTypes;
        
        for (auto v : str->getItems()) {
            Type *t = translateType(v.type);
            elementTypes.push_back(t);
        }
        
        StructType *s = StructType::create(*context, elementTypes);
        s->setName(str->getName());
        
        structTable[str->getName()] = s;
        structElementTypeTable[str->getName()] = elementTypes;
    }

    // Build all other functions
    for (auto global : tree->getGlobalStatements()) {
        switch (global->getType()) {
            case V_AstType::Func: {
                symtable.clear();
                typeTable.clear();
                
                compileFunction(global);
            } break;
            
            case V_AstType::ExternFunc: {
                compileExternFunction(global);
            } break;

            default: {}
        }
    }
}

void Compiler::debug() {
    mod->print(errs(), nullptr);
}

void Compiler::emitLLVM(std::string path) {
    std::error_code errorCode;
    raw_fd_ostream writer(path, errorCode);
    
    mod->print(writer, NULL);
}

// Compiles an individual statement
void Compiler::compileStatement(AstStatement *stmt) {
    switch (stmt->getType()) {
        // Expression statement
        case V_AstType::ExprStmt: {
            AstExprStatement *expr_stmt = static_cast<AstExprStatement *>(stmt);
            compileValue(expr_stmt->getExpression());
        } break;
    
        // A variable declaration (alloca) statement
        case V_AstType::VarDec: {
            AstVarDec *vd = static_cast<AstVarDec *>(stmt);
            Type *type = translateType(vd->getDataType());
            
            AllocaInst *var = builder->CreateAlloca(type);
            symtable[vd->getName()] = var;
            typeTable[vd->getName()] = vd->getDataType();
        } break;
        
        // A structure declaration
        case V_AstType::StructDec: compileStructDeclaration(stmt); break;
        
        // Function call statements
        case V_AstType::FuncCallStmt: {
            compileFuncCallStatement(stmt);
        } break;
        
        // A return statement
        case V_AstType::Return: {
            compileReturnStatement(stmt);
        } break;
        
        // An IF statement
        case V_AstType::If: {
            compileIfStatement(stmt);
        } break;
        
        // A while loop
        case V_AstType::While: {
            compileWhileStatement(stmt);
        } break;
        
        // A break statement
        case V_AstType::Break: {
            builder->CreateBr(breakStack.top());
        } break;
        
        // A continue statement
        case V_AstType::Continue: {
            builder->CreateBr(continueStack.top());
        } break;
        
        default: {}
    }
}

// Converts an AST value to an LLVM value
Value *Compiler::compileValue(AstExpression *expr, bool isAssign) {
    switch (expr->getType()) {
        case V_AstType::I8L: {
            AstI8 *i8 = static_cast<AstI8 *>(expr);
            return builder->getInt8(i8->getValue());
        } break;
        
        case V_AstType::I16L: {
            AstI16 *i16 = static_cast<AstI16 *>(expr);
            return builder->getInt16(i16->getValue());
        } break;
        
        case V_AstType::I32L: {
            AstI32 *ival = static_cast<AstI32 *>(expr);
            return builder->getInt32(ival->getValue());
        } break;
        
        case V_AstType::I64L: {
            AstI64 *i64 = static_cast<AstI64 *>(expr);
            return builder->getInt64(i64->getValue());
        } break;
        
        case V_AstType::CharL: {
            AstChar *cval = static_cast<AstChar *>(expr);
            return builder->getInt8(cval->getValue());
        } break;
        
        case V_AstType::StringL: {
            AstString *str = static_cast<AstString *>(expr);
            return builder->CreateGlobalStringPtr(str->getValue());
        } break;
        
        case V_AstType::ID: {
            AstID *id = static_cast<AstID *>(expr);
            AllocaInst *ptr = symtable[id->getValue()];
            Type *type = translateType(typeTable[id->getValue()]);
            
            if (typeTable[id->getValue()]->getType() == V_AstType::Struct || isAssign) return ptr;
            return builder->CreateLoad(type, ptr);
        } break;
        
        case V_AstType::ArrayAccess: {
            AstArrayAccess *acc = static_cast<AstArrayAccess *>(expr);
            AllocaInst *ptr = symtable[acc->getValue()];
            AstDataType *ptrType = typeTable[acc->getValue()];
            Value *index = compileValue(acc->getIndex());
            
            if (ptrType->getType() == V_AstType::String) {
                PointerType *strPtrType = Type::getInt8PtrTy(*context);
                Type *i8Type = Type::getInt8Ty(*context);
                
                Value *arrayPtr = builder->CreateLoad(strPtrType, ptr);
                Value *ep = builder->CreateGEP(i8Type, arrayPtr, index);
                if (isAssign) return ep;
                else return builder->CreateLoad(i8Type, ep);
            } else {
                AstDataType *subType = static_cast<AstPointerType *>(ptrType)->getBaseType();
                Type *arrayPtrType = translateType(ptrType);
                Type *arrayElementType = translateType(subType);
                
                Value *ptrLd = builder->CreateLoad(arrayPtrType, ptr);
                Value *ep = builder->CreateGEP(arrayElementType, ptrLd, index);
                if (isAssign) return ep;
                else return builder->CreateLoad(arrayElementType, ep);
            }
        } break;

        case V_AstType::StructAccess: return compileStructAccess(expr, isAssign);
        
        case V_AstType::FuncCallExpr: {
            AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(expr);
            std::vector<Value *> args;
            
            AstExprList *list = static_cast<AstExprList *>(fc->getArgExpression());
            for (auto arg : list->getList()) {
                Value *val = compileValue(arg);
                args.push_back(val);
            }
            
            Function *callee = mod->getFunction(fc->getName());
            if (!callee) std::cerr << "Invalid function call statement." << std::endl;
            return builder->CreateCall(callee, args);
        } break;
        
        case V_AstType::Neg: {
            AstNegOp *op = static_cast<AstNegOp *>(expr);
            Value *val = compileValue(op->getVal());
            
            return builder->CreateNeg(val);
        } break;
        
        case V_AstType::Assign: {
            AstAssignOp *op = static_cast<AstAssignOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            
            Value *ptr = compileValue(lvalExpr, true);
            Value *rval = compileValue(op->getRVal());
            
            builder->CreateStore(rval, ptr);
        } break;
        
        case V_AstType::LogicalAnd:
        case V_AstType::LogicalOr: {
            AstBinaryOp *op = static_cast<AstBinaryOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            AstExpression *rvalExpr = op->getRVal();
            
            // We only want the LVal first
            Value *lval = compileValue(lvalExpr);
            
            // Create the blocks
            BasicBlock *trueBlock = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
            ++blockCount;
            
            BasicBlock *current = builder->GetInsertBlock();
            trueBlock->moveAfter(current);
            
            // Create the conditional branch
            if (expr->getType() == V_AstType::LogicalAnd) {
                BasicBlock *falseBlock = logicalAndStack.top();
                builder->CreateCondBr(lval, trueBlock, falseBlock);
            } else if (expr->getType() == V_AstType::LogicalOr) {
                BasicBlock *trueBlock1 = logicalOrStack.top();
                builder->CreateCondBr(lval, trueBlock1, trueBlock);
            }
            
            // Now, build the body of the second block
            builder->SetInsertPoint(trueBlock);
            return compileValue(rvalExpr);
        } break;
        
        case V_AstType::Add:
        case V_AstType::Sub: 
        case V_AstType::Mul:
        case V_AstType::Div:
        case V_AstType::Mod:
        case V_AstType::And:
        case V_AstType::Or:
        case V_AstType::Xor:
        case V_AstType::EQ:
        case V_AstType::NEQ:
        case V_AstType::GT:
        case V_AstType::LT:
        case V_AstType::GTE:
        case V_AstType::LTE: {
            AstBinaryOp *op = static_cast<AstBinaryOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            AstExpression *rvalExpr = op->getRVal();
            
            Value *lval = compileValue(lvalExpr);
            Value *rval = compileValue(rvalExpr);
            
            bool strOp = false;
            bool rvalStr = false;
            
            if (lvalExpr->getType() == V_AstType::StringL || rvalExpr->getType() == V_AstType::StringL) {
                strOp = true;
                rvalStr = true;
            } else if (lvalExpr->getType() == V_AstType::StringL && rvalExpr->getType() == V_AstType::CharL) {
                strOp = true;
            } else if (lvalExpr->getType() == V_AstType::ID && rvalExpr->getType() == V_AstType::CharL) {
                AstID *lvalID = static_cast<AstID *>(lvalExpr);
                if (typeTable[lvalID->getValue()]->getType() == V_AstType::String) strOp = true;
            } else if (lvalExpr->getType() == V_AstType::ID && rvalExpr->getType() == V_AstType::ID) {
                AstID *lvalID = static_cast<AstID *>(lvalExpr);
                AstID *rvalID = static_cast<AstID *>(rvalExpr);
                
                if (typeTable[lvalID->getValue()]->getType() == V_AstType::String) strOp = true;
                if (typeTable[rvalID->getValue()]->getType() == V_AstType::String) {
                    strOp = true;
                    rvalStr = true;
                } else if (typeTable[rvalID->getValue()]->getType() == V_AstType::Char ||
                           typeTable[rvalID->getValue()]->getType() == V_AstType::Int8) {
                    strOp = true;          
                }
            }
            
            // Build a string comparison if necessary
            if (strOp) {
                std::vector<Value *> args;
                args.push_back(lval);
                args.push_back(rval);
            
                if (op->getType() == V_AstType::EQ || op->getType() == V_AstType::NEQ) {
                    Function *strcmp = mod->getFunction("stringcmp");
                    if (!strcmp) std::cerr << "Error: Corelib function \"stringcmp\" not found." << std::endl;
                    Value *strcmpCall = builder->CreateCall(strcmp, args);
                    
                    int cmpVal = 0;
                    if (op->getType() == V_AstType::NEQ) cmpVal = 0;
                    Value *cmpValue = builder->getInt32(cmpVal);
                    
                    return builder->CreateICmpEQ(strcmpCall, cmpValue);
                } else if (op->getType() == V_AstType::Add) {
                    if (rvalStr) {
                        Function *callee = mod->getFunction("strcat_str");
                        if (!callee) std::cerr << "Error: corelib function \"strcat_str\" not found." << std::endl;
                        return builder->CreateCall(callee, args);
                    } else {
                        Function *callee = mod->getFunction("strcat_char");
                        if (!callee) std::cerr << "Error: corelib function \"strcat_char\" not found." << std::endl;
                        return builder->CreateCall(callee, args);
                    }
                } else {
                    // Invalid
                    return nullptr;
                }
            }
            
            // Otherwise, build a normal comparison
            switch (expr->getType()) {
                case V_AstType::Add: return builder->CreateAdd(lval, rval);
                case V_AstType::Sub: return builder->CreateSub(lval, rval);
                case V_AstType::Mul: return builder->CreateMul(lval, rval);
                case V_AstType::Div: return builder->CreateSDiv(lval, rval);
                case V_AstType::Mod: return builder->CreateSRem(lval, rval);
                
                case V_AstType::And: return builder->CreateAnd(lval, rval);
                case V_AstType::Or:  return builder->CreateOr(lval, rval);
                case V_AstType::Xor: return builder->CreateXor(lval, rval);
                    
                case V_AstType::EQ: return builder->CreateICmpEQ(lval, rval);
                case V_AstType::NEQ: return builder->CreateICmpNE(lval, rval);
                case V_AstType::GT: return builder->CreateICmpSGT(lval, rval);
                case V_AstType::LT: return builder->CreateICmpSLT(lval, rval);
                case V_AstType::GTE: return builder->CreateICmpSGE(lval, rval);
                case V_AstType::LTE: return builder->CreateICmpSLE(lval, rval);
                    
                default: {}
            }
        } break;
        
        default: {}
    }
    
    return nullptr;
}

Type *Compiler::translateType(AstDataType *dataType) {
    Type *type;
    
    switch (dataType->getType()) {
        case V_AstType::Void: type = Type::getVoidTy(*context); break;
        case V_AstType::Bool: type = Type::getInt32Ty(*context); break;
        case V_AstType::Char:
        case V_AstType::Int8: type = Type::getInt8Ty(*context); break;
        case V_AstType::Int16: type = Type::getInt16Ty(*context); break;
        case V_AstType::Int32: type = Type::getInt32Ty(*context); break;
        case V_AstType::Int64: type = Type::getInt64Ty(*context); break;
        case V_AstType::String:  type = PointerType::getUnqual(Type::getInt8PtrTy(*context)); break;
        
        case V_AstType::Ptr: {
            AstPointerType *ptrType = static_cast<AstPointerType *>(dataType);
            Type *baseType = translateType(ptrType->getBaseType());
            type = PointerType::getUnqual(baseType);
        } break;
        
        case V_AstType::Struct: {
            AstStructType *sType = static_cast<AstStructType *>(dataType);
            type = structTable[sType->getName()];
        } break;
        
        default: {}
    }
    
    return type;
}

int Compiler::getStructIndex(std::string name, std::string member) {
    std::string name2 = structVarTable[name];
    if (name2 != "") name = name2;
    
    for (auto s : tree->getStructs()) {
        if (s->getName() != name) continue;

        std::vector<Var> members = s->getItems();
        for (int i = 0; i<members.size(); i++) {
            if (members.at(i).name == member) return i;
        }
    }

    return 0;
}

