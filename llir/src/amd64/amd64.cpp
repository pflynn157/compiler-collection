//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <amd64/amd64.hpp>
#include <llir.hpp>

namespace LLIR {

Amd64Writer::Amd64Writer(Module *mod) {
    this->mod = mod;
    file = new X86File(mod->getName());
    
    // Init the register map
    regMap[-1] = X86Reg::R15;
    regMap[-2] = X86Reg::R14;
    regMap[0] = X86Reg::AX;
    regMap[1] = X86Reg::BX;
    regMap[2] = X86Reg::CX;
    regMap[3] = X86Reg::DX;
    
    // Init the arguments register map
    argRegMap[0] = X86Reg::DI;
    argRegMap[1] = X86Reg::SI;
    argRegMap[2] = X86Reg::DX;
    argRegMap[3] = X86Reg::CX;
    argRegMap[4] = X86Reg::R8;
    argRegMap[5] = X86Reg::R9;
}

Amd64Writer::~Amd64Writer() {
    delete file;
}

void Amd64Writer::compile() {
    // Data section
    for (int i = 0; i<mod->getStringCount(); i++) {
        StringPtr *str = mod->getString(i);
        X86Data *d = new X86Data(str->getName(), str->getValue());
        file->addData(d);
    }
    
    // Text section
    for (int i = 0; i<mod->getFunctionCount(); i++) {
        Function *func = mod->getFunction(i);
        switch (func->getLinkage()) {
            case Linkage::Global: {
                X86GlobalFunc *x86Func = new X86GlobalFunc(func->getName());
                file->addCode(x86Func);
            } break;
            
            case Linkage::Local: {} break;
            
            case Linkage::Extern: {
                //assembly += ".extern " + func->getName() + "\n";
                continue;
            } break;
        }
        
        // Setup the stack
        X86Imm *stackImm = new X86Imm(0);
        
        X86Push *p = new X86Push(new X86Reg64(X86Reg::BP));
        file->addCode(p);
        X86Mov *mov = new X86Mov(new X86Reg64(X86Reg::BP), new X86Reg64(X86Reg::SP));
        file->addCode(mov);
        X86Sub *sub = new X86Sub(new X86Reg64(X86Reg::SP), stackImm);
        file->addCode(sub);
        
        // Blocks
        std::string prefix = "F" + std::to_string(i) + "_";
        
        for (int j = 0; j<func->getBlockCount(); j++) {
            Block *block = func->getBlock(j);
            if (j != 0) {
                file->addCode(new X86Label(prefix + block->getName()));
            }
            
            // Instructions
            for (int k = 0; k<block->getInstrCount(); k++) {
                compileInstruction(block->getInstruction(k), prefix);
            }
        }
        
        if (stackPos < 16) {
            stackImm->setValue(16);
        } else {
            int i = 16;
            for (; i < (stackPos + 4); i += 16) {}
            i += 16;
            stackImm->setValue(i);
        }
        stackPos = 0;
        
        // Clean up the stack and leave
        //file->addCode(new X86Leave);
        //file->addCode(new X86Ret);
    }
}

void Amd64Writer::compileInstruction(Instruction *instr, std::string prefix) {
    switch (instr->getType()) {
        case InstrType::None: break;
        
        case InstrType::Ret: {
            if (instr->getDataType()->getType() == DataType::Void) {
                file->addCode(new X86Leave);
                file->addCode(new X86Ret);
                break;
            }
        
            Type *type = instr->getDataType();
            X86Operand *src = compileOperand(instr->getOperand1(), type, prefix);
            X86Operand *dest;
            switch (type->getType()) {
                case DataType::I8: dest = new X86Reg8(X86Reg::AX); break;
                case DataType::I16: dest = new X86Reg16(X86Reg::AX); break;
                case DataType::I32: dest = new X86Reg32(X86Reg::AX); break;
                case DataType::Struct:
                case DataType::Ptr:
                case DataType::I64: dest = new X86Reg64(X86Reg::AX); break;
                
                default: {}
            }
            
            X86Mov *mov = new X86Mov(dest, src);
            file->addCode(mov);
            
            file->addCode(new X86Leave);
            file->addCode(new X86Ret);
        } break;
        
        case InstrType::RetVoid: {
            file->addCode(new X86Leave);
            file->addCode(new X86Ret);
        } break;
        
        // Math
        case InstrType::Add:
        case InstrType::Sub:
        case InstrType::And:
        case InstrType::Or:
        case InstrType::Xor: {
            X86Operand *op1 = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *op2 = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            X86Operand *fop1, *fop2;
            if (op1->getType() == X86Type::Imm) {
                fop2 = op1;
                fop1 = op2;
            } else {
                fop1 = op1;
                fop2 = op2;
            }
            
            X86Instr *instr2;
            switch (instr->getType()) {
                case InstrType::Add: instr2 = new X86Add(fop1, fop2); break;
                case InstrType::Sub: instr2 = new X86Sub(fop1, fop2); break;
                case InstrType::And: instr2 = new X86And(fop1, fop2); break;
                case InstrType::Or: instr2 = new X86Or(fop1, fop2); break;
                case InstrType::Xor: instr2 = new X86Xor(fop1, fop2); break;
                
                default: {}
            }
            
            file->addCode(instr2);
            
            // Now, we need a move so we're in the right register
            // I love x86
            X86Operand *dest = compileOperand(instr->getDest(), instr->getDataType(), prefix);
            X86Mov *mov = new X86Mov(dest, fop1);
            file->addCode(mov);
        } break;
        
        // Multiplication
        // This is like the only instruction that makes sense with the three operands
        case InstrType::UMul:
        case InstrType::SMul: {
            X86Operand *op1 = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *op2 = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            X86Operand *dest = compileOperand(instr->getDest(), instr->getDataType(), prefix);
            X86Operand *fop1, *fop2;
            if (op1->getType() == X86Type::Imm) {
                fop2 = op1;
                fop1 = op2;
            } else {
                fop1 = op1;
                fop2 = op2;
            }
            
            X86IMul *imul = new X86IMul(dest, op1, op2);
            file->addCode(imul);
        } break;
        
        // Division
        // This operand is even more interesting!
        case InstrType::UDiv:
        case InstrType::SDiv:
        case InstrType::URem:
        case InstrType::SRem: {
            X86Operand *op1 = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *op2 = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            X86Operand *dest = compileOperand(instr->getDest(), instr->getDataType(), prefix);
            X86Operand *rax = compileOperand(new HReg(0), instr->getDataType(), prefix);
            X86Operand *fop2;
            bool pop = false;
            if (op2->getType() == X86Type::Imm) {
                X86Operand *fop2_long = compileOperand(new HReg(-1), Type::createI64Type(), prefix);
                fop2 = compileOperand(new HReg(-1), instr->getDataType(), prefix);
                X86Mov *mov1 = new X86Mov(fop2, op2);
                file->addCode(mov1);
                pop = true;
            } else {
                fop2 = op2;
            }
            
            X86Mov *mov = new X86Mov(op1, rax);
            file->addCode(mov);
            
            X86Cdq *cdq = new X86Cdq;
            file->addCode(cdq);
            
            X86IDiv *idiv = new X86IDiv(fop2);
            file->addCode(idiv);
            
            X86Mov *mov2 = new X86Mov(dest, rax);
            file->addCode(mov2);
        } break;
        
        case InstrType::Br: {
            X86Operand *label = compileOperand(instr->getOperand1(), nullptr, prefix);
            X86Jmp *jmp = new X86Jmp(label, X86Type::Jmp);
            file->addCode(jmp);
        } break;
        
        // All conditional branches
        case InstrType::Beq:
        case InstrType::Bne:
        case InstrType::Bgt:
        case InstrType::Blt:
        case InstrType::Bge:
        case InstrType::Ble: {
            X86Operand *op1 = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *op2 = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            X86Cmp *cmp = new X86Cmp(op1, op2);
            file->addCode(cmp);
            
            X86Operand *label = compileOperand(instr->getOperand3(), nullptr, prefix);
            X86Instr *jmp;
            switch (instr->getType()) {
                case InstrType::Beq: jmp = new X86Jmp(label, X86Type::Je); break;
                case InstrType::Bne: jmp = new X86Jmp(label, X86Type::Jne); break;
                case InstrType::Bgt: jmp = new X86Jmp(label, X86Type::Jg); break;
                case InstrType::Blt: jmp = new X86Jmp(label, X86Type::Jl); break;
                case InstrType::Bge: jmp = new X86Jmp(label, X86Type::Jge); break;
                case InstrType::Ble: jmp = new X86Jmp(label, X86Type::Jle); break;
                
                default: {}
            }
            
            file->addCode(jmp);
        } break;
        
        case InstrType::Call: {
            FunctionCall *fc = static_cast<FunctionCall *>(instr);
            Function *callee = mod->getFunctionByName(fc->getName());
            
            int pos = 0;
            for (Operand *arg : fc->getArgs()) {
                // TODO: Some better argument detection for the registers would be ideal
                Type *argType = Type::createI32Type();
                if (pos < callee->getArgCount()) argType = callee->getArgType(pos);
                X86Operand *op = compileOperand(arg, argType, prefix);
                
                X86Reg regType = argRegMap[pos];
                ++pos;
                
                X86Operand *dest = new X86Reg32(regType);
                switch (argType->getType()) {
                    case DataType::Void: break;
                    case DataType::I8:
                    case DataType::I16:
                    case DataType::I32: dest = new X86Reg32(regType); break;
                    case DataType::Struct:
                    case DataType::Ptr:
                    case DataType::I64: dest = new X86Reg64(regType); break;
                    case DataType::F32:
                    case DataType::F64: break;
                }
                if (op->getType() == X86Type::String)
                    dest = new X86Reg64(regType);
                    
                if (argType->getType() == DataType::Ptr) {
                    PointerType *ptr = static_cast<PointerType *>(argType);
                    if (ptr->getBaseType()->getType() == DataType::Struct) {
                        X86Lea *lea = new X86Lea(dest, op);
                        file->addCode(lea);
                    } else {
                        X86Mov *mov = new X86Mov(dest, op);
                        file->addCode(mov);
                    }
                } else {
                    if (op->getType() == X86Type::Reg8 || op->getType() == X86Type::Reg16) {
                        X86Movsx *mov = new X86Movsx(dest, op);
                        file->addCode(mov);
                    } else {
                        X86Mov *mov = new X86Mov(dest, op);
                        file->addCode(mov);
                    }
                }
            }
            
            X86Call *call = new X86Call(fc->getName());
            file->addCode(call);
        } break;
        
        case InstrType::Alloca: {
            if (instr->getDataType()->getType() == DataType::Struct) {
                StructType *type = static_cast<StructType *>(instr->getDataType());
                for (Type *t : type->getElementTypes()) stackPos += getIntSizeForType(t);
            } else {
                stackPos += getIntSizeForType(instr->getDataType());
            }
            
            Mem *mem = static_cast<Mem *>(instr->getDest());
            memMap[mem->getName()] = stackPos;
        } break;
        
        case InstrType::StructLoad: {
            // First, compile the operands
            X86Operand *src = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            int position = static_cast<Imm *>(instr->getOperand2())->getValue();
            
            // Now, calculate the element position
            StructType *type = static_cast<StructType *>(instr->getDataType());
            Type *elementType = type->getElementTypes().at(position);
            position *= getIntSizeForType(elementType);
            
            // Update the source with the correct position
            X86Mem *mem = static_cast<X86Mem *>(src);
            mem->setSizeAttr(getSizeForType(elementType));
            X86Imm *offset = static_cast<X86Imm *>(mem->getOffset());
            offset->setValue(offset->getValue() + position);
            
            // Now, do the moves
            X86Operand *dest = compileOperand(instr->getDest(), elementType, prefix);
            X86Mov *mov = new X86Mov(dest, mem);
            file->addCode(mov);
        } break;
        
        case InstrType::Load: {
            X86Operand *src = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *dest = compileOperand(instr->getDest(), instr->getDataType(), prefix);
            X86Mov *mov = new X86Mov(dest, src);
            file->addCode(mov);
        } break;
        
        case InstrType::GEP: {
            X86Operand *src = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *index = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            
            // Check the index, and calculate the proper offset
            int offset = 1;
            Type *type = instr->getDataType();
            if (type->getType() == DataType::Ptr) {
                type = static_cast<PointerType *>(type)->getBaseType();
            }
            switch (type->getType()) {
                case DataType::Void: break;
                case DataType::I8: break;
                case DataType::I16: offset = 2; break;
                case DataType::F32:
                case DataType::I32: offset = 4; break;
                case DataType::I64:
                case DataType::F64:
                case DataType::Ptr: offset = 8; break;
            }
                
            if (index->getType() == X86Type::Imm) {
                X86Imm *indexImm = static_cast<X86Imm *>(index);
                int val = indexImm->getValue();
                indexImm->setValue(val * offset);
                index = indexImm;
            } else {
                X86IMul *mul = new X86IMul(index, index, new X86Imm(offset));
                file->addCode(mul);
            }
            
            X86Add *add = new X86Add(src, index);
            file->addCode(add);
            
            // The destination needs to be converted to a regular register
            X86RegPtr *dest = static_cast<X86RegPtr *>(compileOperand(instr->getDest(), instr->getDataType(), prefix));
            X86Reg64 *dest2 = new X86Reg64(dest->getType());
            X86Mov *mov = new X86Mov(dest2, src);
            file->addCode(mov);
        } break;
        
        case InstrType::StructStore: {
            // First, compile the operands
            X86Operand *src = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            int position = static_cast<Imm *>(instr->getOperand2())->getValue();
            
            // Now, calculate the element position
            StructType *type = static_cast<StructType *>(instr->getDataType());
            Type *elementType = type->getElementTypes().at(position);
            position *= getIntSizeForType(elementType);
            
            // Update the source with the correct position
            X86Mem *mem = static_cast<X86Mem *>(src);
            mem->setSizeAttr(getSizeForType(elementType));
            X86Imm *offset = static_cast<X86Imm *>(mem->getOffset());
            offset->setValue(offset->getValue() + position);
            
            // Now, do the moves
            X86Operand *dest = compileOperand(instr->getOperand3(), elementType, prefix);
            X86Mov *mov = new X86Mov(mem, dest);
            file->addCode(mov);
        } break;
        
        case InstrType::Store: {
            X86Operand *src = compileOperand(instr->getOperand1(), instr->getDataType(), prefix);
            X86Operand *dest = compileOperand(instr->getOperand2(), instr->getDataType(), prefix);
            X86Mov *mov = new X86Mov(dest, src);
            file->addCode(mov);
        } break;
    }
}

X86Operand *Amd64Writer::compileOperand(Operand *src, Type *type, std::string prefix) {
    switch (src->getType()) {
        // Return an immediate operand
        case OpType::Imm: {
            Imm *imm = static_cast<Imm *>(src);
            return new X86Imm(imm->getValue());
        }
        
        // Return a memory operand
        case OpType::Mem: {
            Mem *mem = static_cast<Mem *>(src);
            int pos = memMap[mem->getName()];
            X86Mem *mem2 = new X86Mem(new X86Imm(0 - pos));
            mem2->setSizeAttr(getSizeForType(type));
            return mem2;
        }
        
        // Return a hardware register
        case OpType::HReg: {
            HReg *reg = static_cast<HReg *>(src);
            X86Reg rType = regMap[reg->getNum()];
            
            switch (type->getType()) {
                case DataType::Void: break;
                case DataType::I8: return new X86Reg8(rType);
                case DataType::I16: return new X86Reg16(rType);
                case DataType::I32: return new X86Reg32(rType);
                case DataType::Struct:
                case DataType::Ptr:
                case DataType::I64: return new X86Reg64(rType);
                case DataType::F32:
                case DataType::F64: break;
            }
        } break;
        
        // Return an argument register
        case OpType::AReg: {
            AReg *reg = static_cast<AReg *>(src);
            X86Reg rType = argRegMap[reg->getNum()];
            
            switch (type->getType()) {
                case DataType::Void: break;
                case DataType::I8: return new X86Reg8(rType);
                case DataType::I16: return new X86Reg16(rType);
                case DataType::I32: return new X86Reg32(rType);
                case DataType::Struct:
                case DataType::Ptr:
                case DataType::I64: return new X86Reg64(rType);
                case DataType::F32:
                case DataType::F64: break;
            }
        } break;
        
        // Return a pointer register
        case OpType::PReg: {
            PReg *reg = static_cast<PReg *>(src);
            X86Reg rType = regMap[reg->getNum()];
            X86RegPtr *reg2 = new X86RegPtr(rType);
            reg2->setSizeAttr(getSizeForType(type));
            return reg2;
        } break;
        
        // A string operand
        case OpType::String: {
            StringPtr *ptr = static_cast<StringPtr *>(src);
            return new X86String(ptr->getName());
        }
        
        // A label reference
        case OpType::Label: {
            Label *lbl = static_cast<Label *>(src);
            return new X86LabelRef(prefix + lbl->getName());
        }
        
        default: return nullptr;
    }
    
    return nullptr;
}

void Amd64Writer::dump() {
    std::cout << file->print() << std::endl;
}

void Amd64Writer::writeToFile() {
    std::string path = "/tmp/" + mod->getName() + ".s";
    std::ofstream writer(path);
    writer << file->print() << std::endl;
    writer.close();
}

void Amd64Writer::writeToFile(std::string path) {
    std::ofstream writer(path);
    writer << file->print() << std::endl;
    writer.close();
}

// TODO: This function probably should be replaced
void Amd64Writer::build() {
    std::string asmPath = "/tmp/" + mod->getName() + ".s";
    std::string objPath = "/tmp/" + mod->getName() + ".o";
    std::string binPath = "/tmp/" + mod->getName();
    
    std::string cmd1 = "as " + asmPath + " -o " + objPath;
    std::string cmd2 = "gcc " + objPath + " -o " + binPath;
    
    system(cmd1.c_str());
    system(cmd2.c_str());
}

std::string Amd64Writer::getSizeForType(Type *type) {
    switch (type->getType()) {
        case DataType::Void: break;
        case DataType::I8: return "BYTE PTR";
        case DataType::I16: return "WORD PTR";
        case DataType::F32:
        case DataType::I32: return "DWORD PTR";
        case DataType::I64:
        case DataType::F64:
        case DataType::Ptr: return "QWORD PTR";
    }
    return "";
}

int Amd64Writer::getIntSizeForType(Type *type) {
    switch (type->getType()) {
        case DataType::Void: break;
        case DataType::I8: return 1;
        case DataType::I16: return 2;
        case DataType::F32:
        case DataType::I32: return 4;
        case DataType::I64:
        case DataType::F64:
        case DataType::Ptr: return 8;
    }
    return 0;
}

} // end namespace LLIR
