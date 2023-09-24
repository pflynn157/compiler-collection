//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//

#include <iostream>
#include <exception>
#include <algorithm>
#include <memory>

#include "Compiler.hpp"

Compiler::Compiler(std::shared_ptr<AstTree> tree, CFlags cflags) {
    this->tree = tree;
    this->cflags = cflags;

    mod = new LLIR::Module(cflags.name);
    builder = new LLIR::IRBuilder(mod);
}

void Compiler::compile() {
    // Build the structures used by the program
    for (auto str : tree->structs) {
        std::vector<LLIR::Type *> elementTypes;
        
        for (auto v : str->items) {
            LLIR::Type *t = translateType(v.type);
            elementTypes.push_back(t);
        }
        
        LLIR::StructType *s = new LLIR::StructType(str->name, elementTypes);
        structTable[str->name] = s;
        structElementTypeTable[str->name] = elementTypes;
    }

    // Build all other functions
    for (auto global : tree->global_statements) {
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
    //mod->print(errs(), nullptr);
    mod->print();
}

// Compiles an individual statement
void Compiler::compileStatement(std::shared_ptr<AstStatement> stmt) {
    switch (stmt->type) {
        // Expression statement
        case V_AstType::ExprStmt: {
            auto expr_stmt = std::static_pointer_cast<AstExprStatement>(stmt);
            compileValue(expr_stmt->expression, expr_stmt->dataType);
        } break;
    
        // A variable declaration (alloca) statement
        case V_AstType::VarDec: {
            auto vd = std::static_pointer_cast<AstVarDec>(stmt);
            LLIR::Type *type = translateType(vd->data_type);
            
            LLIR::Reg *var = builder->createAlloca(type);
            symtable[vd->name] = var;
            typeTable[vd->name] = vd->data_type;
        } break;
        
        // A structure declaration
        case V_AstType::StructDec: {
            auto sd = std::static_pointer_cast<AstStructDec>(stmt);
            LLIR::StructType *type = structTable[sd->struct_name];
            
            if (sd->no_init) {
                LLIR::PointerType *typePtr = new LLIR::PointerType(type);
                LLIR::Reg *alloca = builder->createAlloca(typePtr);
                symtable[sd->var_name] = alloca;
                //typeTable[sd->getVarName()] = V_AstType::Ptr;
                structVarTable[sd->var_name] = sd->struct_name;
                structArgs.push_back(sd->var_name);            // This is a hack. Aka, it sucks
            } else {
                LLIR::Reg *var = builder->createAlloca(type);
                symtable[sd->var_name] = var;
                //typeTable[sd->getVarName()] = V_AstType::Struct;
                structVarTable[sd->var_name] = sd->struct_name;
                
                std::shared_ptr<AstStruct> str = nullptr;
                for (auto const &s : tree->structs) {
                    if (s->name == sd->struct_name) {
                        str = s;
                        break;
                    }
                }
                if (str == nullptr) return;
                
                // Init the elements
                int index = 0;
                for (Var member : str->items) {
                    std::shared_ptr<AstExpression> defaultExpr = str->default_expressions[member.name];
                    LLIR::Operand *defaultVal = compileValue(defaultExpr, member.type);
                    
                    builder->createStructStore(type, var, index, defaultVal);
                    
                    ++index;
               }
            }
        } break;
        
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
            builder->createBr(breakStack.top());
        } break;
        
        // A continue statement
        case V_AstType::Continue: {
            builder->createBr(continueStack.top());
        } break;
        
        default: {}
    }
}

// Converts an AST value to an LLVM value
LLIR::Operand *Compiler::compileValue(std::shared_ptr<AstExpression> expr, std::shared_ptr<AstDataType> dataType, LLIR::Block *destBlock, bool isAssign) {
    LLIR::Type *type = translateType(dataType);

    switch (expr->type) {
        case V_AstType::I8L: {
            auto i8 = std::static_pointer_cast<AstI8>(expr);
            return builder->createI8(i8->value);
        } break;
        
        case V_AstType::I16L: {
            auto i16 = std::static_pointer_cast<AstI16>(expr);
            return builder->createI16(i16->value);
        } break;
        
        case V_AstType::I32L: {
            auto ival = std::static_pointer_cast<AstI32>(expr);
            return builder->createI32(ival->value);
        } break;
        
        case V_AstType::I64L: {
            auto i64 = std::static_pointer_cast<AstI64>(expr);
            return builder->createI64(i64->value);
        } break;
        
        case V_AstType::CharL: {
            auto cval = std::static_pointer_cast<AstChar>(expr);
            return builder->createI8(cval->value);
        } break;
        
        case V_AstType::StringL: {
            auto str = std::static_pointer_cast<AstString>(expr);
            return builder->createString(str->value);
        } break;
        
        case V_AstType::ID: {
            auto id = std::static_pointer_cast<AstID>(expr);
            LLIR::Reg *ptr = symtable[id->value];
            LLIR::Type *type = translateType(typeTable[id->value]);
                        
            //if (typeTable[id->value] == V_AstType::Ptr && ptrTable[id->value] == V_AstType::Struct) {
            //    return ptr;
            //}
            
            //if (typeTable[id->value] == V_AstType::Struct || isAssign) return ptr;
            return builder->createLoad(type, ptr);
        } break;
        
        case V_AstType::ArrayAccess: {
            auto acc = std::static_pointer_cast<AstArrayAccess>(expr);
            LLIR::Reg *ptr = symtable[acc->value];
            std::shared_ptr<AstDataType> ptrType = typeTable[acc->value];
            LLIR::Operand *index = compileValue(acc->index);
            
            if (ptrType->type == V_AstType::String) {
                LLIR::PointerType *strPtrType = LLIR::PointerType::createI8PtrType();
                LLIR::Type *i8Type = LLIR::Type::createI8Type();
                
                LLIR::Operand *ptrLd = builder->createLoad(strPtrType, ptr);
                LLIR::Operand *ep = builder->createGEP(strPtrType, ptrLd, index);
                if (isAssign) return ep;
                return builder->createLoad(i8Type, ep);
            } else {
                LLIR::Type *arrayPtrType = translateType(ptrType);
                //LLIR::Type *arrayElementType = translateType(subType);
                
                //LLIR::Operand *ptrLd = builder->createLoad(arrayPtrType, ptr);
                //LLIR::Operand *ep = builder->createGEP(arrayPtrType, ptrLd, index);
                //if (isAssign) return ep;
                //return builder->createLoad(arrayElementType, ep);
                return nullptr;
            }
        } break;

        case V_AstType::StructAccess: {
            auto sa = std::static_pointer_cast<AstStructAccess>(expr);
            LLIR::Reg *ptr = symtable[sa->var];
            int pos = getStructIndex(sa->var, sa->member);
            
            std::string strTypeName = structVarTable[sa->var];
            LLIR::StructType *strType = structTable[strTypeName];
            LLIR::Type *elementType = structElementTypeTable[strTypeName][pos];
            
            if (std::find(structArgs.begin(), structArgs.end(), sa->var) != structArgs.end()) {
                LLIR::PointerType *strPtrType = new LLIR::PointerType(strType);
                LLIR::PointerType *elementPtrType = new LLIR::PointerType(elementType);
                
                ptr = builder->createLoad(strPtrType, ptr);
                LLIR::Operand *ep = builder->createGEP(elementPtrType, ptr, new LLIR::Imm(pos));
                if (isAssign) return ep;
                return builder->createLoad(elementType, ep);
            } else {
                return builder->createStructLoad(strType, ptr, pos);
            }
        } break;
        
        case V_AstType::FuncCallExpr: {
            auto fc = std::static_pointer_cast<AstFuncCallExpr>(expr);
            std::vector<LLIR::Operand *> args;
            
            auto list = std::static_pointer_cast<AstExprList>(fc->args);
            for (auto arg : list->list) {
                LLIR::Operand *val = compileValue(arg);
                args.push_back(val);
            }
            
            return builder->createCall(type, fc->name, args);
        } break;
        
        case V_AstType::Assign: {
            auto op = std::static_pointer_cast<AstAssignOp>(expr);
            std::shared_ptr<AstExpression> lvalExpr = op->lval;
            
            std::string name = "";
            switch (lvalExpr->type) {
                case V_AstType::ID: name = std::static_pointer_cast<AstID>(lvalExpr)->value; break;
                case V_AstType::ArrayAccess: {
                    name = std::static_pointer_cast<AstArrayAccess>(lvalExpr)->value;
                } break;
                case V_AstType::StructAccess: {
                    name = std::static_pointer_cast<AstStructAccess>(lvalExpr)->var;
                } break;
                
                default: {}
            }
            
            LLIR::Operand *ptr = compileValue(lvalExpr, dataType, nullptr, true);
            LLIR::Operand *rval = compileValue(op->rval, dataType);
            
            //V_AstType ptrType = typeTable[name];
            //LLIR::Type *type = translateType(ptrType);
            //if (ptrType == V_AstType::Struct) {
            //    std::string strTypeName = structVarTable[name];
            //    type = structTable[strTypeName];
            //}
            
            builder->createStore(type, rval, ptr);
        } break;
        
        case V_AstType::Neg: {
            auto op = std::static_pointer_cast<AstNegOp>(expr);
            LLIR::Operand *val = compileValue(op->value, dataType);
            
            return builder->createNeg(type, val);
        } break;
        
        case V_AstType::LogicalAnd:
        case V_AstType::LogicalOr: {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            auto lvalExpr = op->lval;
            auto rvalExpr = op->rval;
            
            // We only want the LVal first
            //LLIR::Operand *lval = compileValue(lvalExpr, dataType);
            
            // Create the blocks
            LLIR::Block *trueBlock = new LLIR::Block("true" + std::to_string(blockCount));
            ++blockCount;
            
            LLIR::Block *current = builder->getInsertPoint();
            builder->addBlockAfter(current, trueBlock);
            
            // Create the conditional branch
            if (expr->type == V_AstType::LogicalAnd) {
                LLIR::Block *falseBlock = logicalAndStack.top();
                LLIR::Operand *lval = compileValue(lvalExpr, dataType, trueBlock);
                builder->createBr(falseBlock);
            } else if (expr->type == V_AstType::LogicalOr) {
                LLIR::Block *trueBlock1 = logicalOrStack.top();
                LLIR::Operand *lval = compileValue(lvalExpr, dataType, trueBlock1);
                builder->createBr(trueBlock);
            }
            
            // Now, build the body of the second block
            builder->setInsertPoint(trueBlock);
            return compileValue(rvalExpr, dataType, destBlock);
        } break;
        
        case V_AstType::Add:
        case V_AstType::Sub: 
        case V_AstType::Mul:
        case V_AstType::Div:
        case V_AstType::And:
        case V_AstType::Or:
        case V_AstType::Xor:
        case V_AstType::EQ:
        case V_AstType::NEQ:
        case V_AstType::GT:
        case V_AstType::LT:
        case V_AstType::GTE:
        case V_AstType::LTE: {
            auto op = std::static_pointer_cast<AstBinaryOp>(expr);
            auto lvalExpr = op->lval;
            auto rvalExpr = op->rval;
            
            // Do a type detection so we don't end up with null comparisons
            std::shared_ptr<AstDataType> dType = dataType;
            if (dataType->type == V_AstType::Void) {
                if (lvalExpr->type == V_AstType::ID) {
                    auto id = std::static_pointer_cast<AstID>(lvalExpr);
                    type = translateType(typeTable[id->value]);
                    dType = typeTable[id->value];
                } else if (lvalExpr->type == V_AstType::ID) {
                    auto id = std::static_pointer_cast<AstID>(rvalExpr);
                    type = translateType(typeTable[id->value]);
                    dType = typeTable[id->value];
                }
            }
            
            // Now, compile the operands
            LLIR::Operand *lval = compileValue(lvalExpr, dType);
            LLIR::Operand *rval = compileValue(rvalExpr, dType);
            
            bool strOp = false;
            bool rvalStr = false;
            
            if (lvalExpr->type == V_AstType::StringL || rvalExpr->type == V_AstType::StringL) {
                strOp = true;
                rvalStr = true;
            } else if (lvalExpr->type == V_AstType::StringL && rvalExpr->type == V_AstType::CharL) {
                strOp = true;
            } else if (lvalExpr->type == V_AstType::ID && rvalExpr->type == V_AstType::CharL) {
                auto lvalID = std::static_pointer_cast<AstID>(lvalExpr);
                if (typeTable[lvalID->value]->type == V_AstType::String) strOp = true;
            } else if (lvalExpr->type == V_AstType::ID && rvalExpr->type == V_AstType::ID) {
                auto lvalID = std::static_pointer_cast<AstID>(lvalExpr);
                auto rvalID = std::static_pointer_cast<AstID>(rvalExpr);
                
                if (typeTable[lvalID->value]->type == V_AstType::String) strOp = true;
                if (typeTable[rvalID->value]->type == V_AstType::String) {
                    strOp = true;
                    rvalStr = true;
                } else if (typeTable[rvalID->value]->type == V_AstType::Char ||
                           typeTable[rvalID->value]->type == V_AstType::Int8) {
                    strOp = true;          
                }
            }
            
            // Build a string comparison if necessary
            if (strOp) {
                std::vector<LLIR::Operand *> args;
                args.push_back(lval);
                args.push_back(rval);
            
                if (op->type == V_AstType::EQ || op->type == V_AstType::NEQ) {
                    LLIR::Operand *strcmpCall = builder->createCall(LLIR::Type::createI32Type(), "stringcmp", args);
                    
                    int cmpVal = 0;
                    if (op->type == V_AstType::NEQ) cmpVal = 0;
                    LLIR::Operand *cmpValue = builder->createI32(cmpVal);
                    
                    return builder->createBeq(LLIR::Type::createI32Type(), strcmpCall, cmpValue, destBlock);
                } else if (op->type == V_AstType::Add) {
                    if (rvalStr) {
                        return builder->createCall(LLIR::PointerType::createI8PtrType(), "strcat_str", args);
                    } else {
                        return builder->createCall(LLIR::PointerType::createI8PtrType(), "strcat_char", args);
                    }
                } else {
                    // Invalid
                    return nullptr;
                }
            }
            
            // Otherwise, build a normal comparison
            switch (expr->type) {
                case V_AstType::Add: return builder->createAdd(type, lval, rval);
                case V_AstType::Sub: return builder->createSub(type, lval, rval);
                case V_AstType::Mul: return builder->createSMul(type, lval, rval);
                case V_AstType::Div: return builder->createSDiv(type, lval, rval);
                
                case V_AstType::And: return builder->createAnd(type, lval, rval);
                case V_AstType::Or:  return builder->createOr(type, lval, rval);
                case V_AstType::Xor: return builder->createXor(type, lval, rval);
                    
                case V_AstType::EQ: return builder->createBeq(type, lval, rval, destBlock);
                case V_AstType::NEQ: return builder->createBne(type, lval, rval, destBlock);
                case V_AstType::GT: return builder->createBgt(type, lval, rval, destBlock);
                case V_AstType::LT: return builder->createBlt(type, lval, rval, destBlock);
                case V_AstType::GTE: return builder->createBge(type, lval, rval, destBlock);
                case V_AstType::LTE: return builder->createBle(type, lval, rval, destBlock);
                    
                default: {}
            }
        } break;
        
        default: {}
    }
    
    return nullptr;
}

LLIR::Type *Compiler::translateType(std::shared_ptr<AstDataType> dataType) {
    if (dataType == nullptr) return nullptr;
    LLIR::Type *type;
    
    switch (dataType->type) {
        case V_AstType::Void: type = LLIR::Type::createVoidType(); break;
        case V_AstType::Bool: type = LLIR::Type::createI32Type(); break;
        case V_AstType::Char:
        case V_AstType::Int8: type = LLIR::Type::createI8Type(); break;
        case V_AstType::Int16: type = LLIR::Type::createI16Type(); break;
        case V_AstType::Int32: type = LLIR::Type::createI32Type(); break;
        case V_AstType::Int64: type = LLIR::Type::createI64Type(); break;
        case V_AstType::String: type = LLIR::PointerType::createI8PtrType(); break;
        
        case V_AstType::Ptr: {
            auto ptrType = std::static_pointer_cast<AstPointerType>(dataType);
            LLIR::Type *baseType = translateType(ptrType->base_type);
            type = new LLIR::PointerType(baseType);
        } break;
        
        case V_AstType::Struct: {
            auto sType = std::static_pointer_cast<AstStructType>(dataType);
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

