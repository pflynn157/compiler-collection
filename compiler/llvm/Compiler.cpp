//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
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

Compiler::Compiler(std::shared_ptr<AstTree> tree, CFlags cflags) {
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
    for (auto str : tree->structs) {
        std::vector<Type *> elementTypes;
        
        for (auto v : str->items) {
            Type *t = translateType(v.type);
            elementTypes.push_back(t);
        }
        
        StructType *s = StructType::create(*context, elementTypes);
        s->setName(str->name);
        
        structTable[str->name] = s;
        structElementTypeTable[str->name] = elementTypes;
    }

    // Build all other functions
    for (auto global : tree->block->getBlock()) {
        switch (global->type) {
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
void Compiler::compileStatement(std::shared_ptr<AstStatement> stmt) {
    switch (stmt->type) {
        // Expression statement
        case V_AstType::ExprStmt: {
            std::shared_ptr<AstExprStatement> expr_stmt = std::static_pointer_cast<AstExprStatement>(stmt);
            compileValue(expr_stmt->expression);
        } break;
    
        // A variable declaration (alloca) statement
        case V_AstType::VarDec: {
            std::shared_ptr<AstVarDec> vd = std::static_pointer_cast<AstVarDec>(stmt);
            Type *type = translateType(vd->data_type);
            
            AllocaInst *var = builder->CreateAlloca(type);
            symtable[vd->name] = var;
            typeTable[vd->name] = vd->data_type;
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
        
        // A repeat loop
        case V_AstType::Repeat: {
            compileRepeatStatement(stmt);
        } break;
        
        // A for loop
        case V_AstType::For: {
            compileForStatement(stmt);
        } break;
        
        // A for-all loop
        case V_AstType::ForAll: {
            compileForAllStatement(stmt);
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
Value *Compiler::compileValue(std::shared_ptr<AstExpression> expr, V_AstType dataType, bool isAssign) {
    if (expr == nullptr) return nullptr;
    switch (expr->type) {
        case V_AstType::I8L: {
            std::shared_ptr<AstI8> i8 = std::static_pointer_cast<AstI8>(expr);
            return builder->getInt8(i8->value);
        } break;
        
        case V_AstType::I16L: {
            std::shared_ptr<AstI16> i16 = std::static_pointer_cast<AstI16>(expr);
            return builder->getInt16(i16->value);
        } break;
        
        case V_AstType::I32L: {
            std::shared_ptr<AstI32> ival = std::static_pointer_cast<AstI32>(expr);
            return builder->getInt32(ival->value);
        } break;
        
        case V_AstType::I64L: {
            std::shared_ptr<AstI64> i64 = std::static_pointer_cast<AstI64>(expr);
            return builder->getInt64(i64->value);
        } break;
        
        case V_AstType::FloatL: {
            std::shared_ptr<AstFloat> flt = std::static_pointer_cast<AstFloat>(expr);
            if (dataType == V_AstType::Float64)
                return ConstantFP::get(Type::getDoubleTy(*context), flt->value);
            return ConstantFP::get(Type::getFloatTy(*context), flt->value);
        } break;
        
        case V_AstType::CharL: {
            std::shared_ptr<AstChar> cval = std::static_pointer_cast<AstChar>(expr);
            return builder->getInt8(cval->value);
        } break;
        
        case V_AstType::StringL: {
            std::shared_ptr<AstString> str = std::static_pointer_cast<AstString>(expr);
            return builder->CreateGlobalStringPtr(str->value);
        } break;
        
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(expr);
            AllocaInst *ptr = symtable[id->value];
            Type *type = translateType(typeTable[id->value]);
            
            if (typeTable[id->value]->type == V_AstType::Struct || isAssign) return ptr;
            return builder->CreateLoad(type, ptr);
        } break;
        
        case V_AstType::ArrayAccess: {
            std::shared_ptr<AstArrayAccess> acc = std::static_pointer_cast<AstArrayAccess>(expr);
            AllocaInst *ptr = symtable[acc->value];
            std::shared_ptr<AstDataType> ptrType = typeTable[acc->value];
            Value *index = compileValue(acc->index);
            
            if (ptrType->type == V_AstType::String) {
                PointerType *strPtrType = Type::getInt8PtrTy(*context);
                Type *i8Type = Type::getInt8Ty(*context);
                
                Value *arrayPtr = builder->CreateLoad(strPtrType, ptr);
                Value *ep = builder->CreateGEP(i8Type, arrayPtr, index);
                if (isAssign) return ep;
                else return builder->CreateLoad(i8Type, ep);
            } else {
                std::shared_ptr<AstDataType> subType = std::static_pointer_cast<AstPointerType>(ptrType)->base_type;
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
            std::shared_ptr<AstFuncCallExpr> fc = std::static_pointer_cast<AstFuncCallExpr>(expr);
            std::vector<Value *> args;
            
            std::shared_ptr<AstExprList> list = std::static_pointer_cast<AstExprList>(fc->args);
            for (auto arg : list->list) {
                Value *val = compileValue(arg);
                args.push_back(val);
            }
            
            Function *callee = mod->getFunction(fc->name);
            if (!callee) std::cerr << "Invalid function call statement." << std::endl;
            return builder->CreateCall(callee, args);
        } break;
        
        case V_AstType::FuncRef: {
            auto ref = std::static_pointer_cast<AstFuncRef>(expr);
            
            Function *callee = mod->getFunction(ref->value);
            if (!callee) std::cerr << "Invalid function reference." << std::endl;
            return builder->CreatePointerCast(callee, PointerType::getUnqual(builder->getVoidTy()));
        }
        
        case V_AstType::PtrTo: {
            auto id = std::static_pointer_cast<AstPtrTo>(expr);
            AllocaInst *ptr = symtable[id->value];
            Type *type = translateType(typeTable[id->value]);
            
            Value *ld1 = builder->CreateLoad(type, ptr);
            return builder->CreateLoad(Type::getInt32Ty(*context), ld1);
        }
        
        case V_AstType::Ref: {
            auto ref = std::static_pointer_cast<AstRef>(expr);
            AllocaInst *ptr2 = symtable[ref->value];
            return ptr2;
        }
        
        case V_AstType::Neg: {
            std::shared_ptr<AstNegOp> op = std::static_pointer_cast<AstNegOp>(expr);
            Value *val = compileValue(op->value);
            
            return builder->CreateNeg(val);
        } break;
        
        case V_AstType::Assign: {
            std::shared_ptr<AstAssignOp> op = std::static_pointer_cast<AstAssignOp >(expr);
            std::shared_ptr<AstExpression> lvalExpr = op->lval;
            
            auto lval_id = std::static_pointer_cast<AstID>(lvalExpr);
            V_AstType dtype = typeTable[lval_id->value]->type;
            
            Value *ptr = compileValue(lvalExpr, V_AstType::Void, true);
            Value *rval = compileValue(op->rval, dtype);
            
            builder->CreateStore(rval, ptr);
        } break;
        
        case V_AstType::LogicalAnd:
        case V_AstType::LogicalOr: {
            std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(expr);
            std::shared_ptr<AstExpression> lvalExpr = op->lval;
            std::shared_ptr<AstExpression> rvalExpr = op->rval;
            
            // We only want the LVal first
            Value *lval = compileValue(lvalExpr);
            
            // Create the blocks
            BasicBlock *trueBlock = BasicBlock::Create(*context, "true" + std::to_string(blockCount), currentFunc);
            ++blockCount;
            
            BasicBlock *current = builder->GetInsertBlock();
            trueBlock->moveAfter(current);
            
            // Create the conditional branch
            if (expr->type == V_AstType::LogicalAnd) {
                BasicBlock *falseBlock = logicalAndStack.top();
                builder->CreateCondBr(lval, trueBlock, falseBlock);
            } else if (expr->type == V_AstType::LogicalOr) {
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
            std::shared_ptr<AstBinaryOp> op = std::static_pointer_cast<AstBinaryOp>(expr);
            std::shared_ptr<AstExpression> lvalExpr = op->lval;
            std::shared_ptr<AstExpression> rvalExpr = op->rval;
            
            Value *lval = compileValue(lvalExpr);
            Value *rval = compileValue(rvalExpr);
            
            bool fltOp = false;
            if (lvalExpr->type == V_AstType::FloatL || rvalExpr->type == V_AstType::FloatL) {
                fltOp = true;
                
                if (lvalExpr->type == V_AstType::ID) {
                    auto lvalID = std::static_pointer_cast<AstID>(lvalExpr);
                    if (typeTable[lvalID->value]->type == V_AstType::Float64)
                        rval = compileValue(rvalExpr, V_AstType::Float64);
                } else if (rvalExpr->type == V_AstType::ID) {
                    auto rvalID = std::static_pointer_cast<AstID>(rvalExpr);
                    if (typeTable[rvalID->value]->type == V_AstType::Float64)
                        lval = compileValue(lvalExpr, V_AstType::Float64);
                }
            } else if (lvalExpr->type == V_AstType::ID && rvalExpr->type == V_AstType::ID) {
                auto lvalID = std::static_pointer_cast<AstID>(lvalExpr);
                auto rvalID = std::static_pointer_cast<AstID>(rvalExpr);
                
                V_AstType lvalType = typeTable[lvalID->value]->type;
                V_AstType rvalType = typeTable[rvalID->value]->type;
                
                if (lvalType == V_AstType::Float32 || lvalType == V_AstType::Float64) fltOp = true;
                if (rvalType == V_AstType::Float32 || rvalType == V_AstType::Float64) fltOp = true;
            }
            
            // Otherwise, build a normal comparison
            if ((dataType == V_AstType::Float32 || dataType == V_AstType::Float64) || fltOp) {
                switch (expr->type) {
                    case V_AstType::Add: return builder->CreateFAdd(lval, rval);
                    case V_AstType::Sub: return builder->CreateFSub(lval, rval);
                    case V_AstType::Mul: return builder->CreateFMul(lval, rval);
                    case V_AstType::Div: return builder->CreateFDiv(lval, rval);
                    
                    case V_AstType::EQ: return builder->CreateFCmpOEQ(lval, rval);
                    case V_AstType::NEQ: return builder->CreateFCmpONE(lval, rval);
                    case V_AstType::GT: return builder->CreateFCmpOGT(lval, rval);
                    case V_AstType::LT: return builder->CreateFCmpOLT(lval, rval);
                    case V_AstType::GTE: return builder->CreateFCmpOGE(lval, rval);
                    case V_AstType::LTE: return builder->CreateFCmpOLE(lval, rval);
                    
                    default: {}
                }
            } else {
                switch (expr->type) {
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
            }
        } break;
        
        default: {}
    }
    
    return nullptr;
}

Type *Compiler::translateType(std::shared_ptr<AstDataType> dataType) {
    if (dataType == nullptr) return Type::getVoidTy(*context);
    Type *type;
    
    switch (dataType->type) {
        case V_AstType::Void: type = Type::getVoidTy(*context); break;
        case V_AstType::Bool: type = Type::getInt32Ty(*context); break;
        case V_AstType::Char:
        case V_AstType::Int8: type = Type::getInt8Ty(*context); break;
        case V_AstType::Int16: type = Type::getInt16Ty(*context); break;
        case V_AstType::Int32: type = Type::getInt32Ty(*context); break;
        case V_AstType::Int64: type = Type::getInt64Ty(*context); break;
        case V_AstType::Float32: type = Type::getFloatTy(*context); break;
        case V_AstType::Float64: type = Type::getDoubleTy(*context); break;
        case V_AstType::String:  type = PointerType::getUnqual(Type::getInt8PtrTy(*context)); break;
        
        case V_AstType::Ptr: {
            std::shared_ptr<AstPointerType> ptrType = std::static_pointer_cast<AstPointerType>(dataType);
            Type *baseType = translateType(ptrType->base_type);
            type = PointerType::getUnqual(baseType);
        } break;
        
        case V_AstType::Struct: {
            std::shared_ptr<AstStructType> sType = std::static_pointer_cast<AstStructType>(dataType);
            type = structTable[sType->name];
        } break;
        
        default: {}
    }
    
    return type;
}

int Compiler::getStructIndex(std::string name, std::string member) {
    std::string name2 = structVarTable[name];
    if (name2 != "") name = name2;
    
    for (auto s : tree->structs) {
        if (s->name != name) continue;

        std::vector<Var> members = s->items;
        for (int i = 0; i<members.size(); i++) {
            if (members.at(i).name == member) return i;
        }
    }

    return 0;
}

