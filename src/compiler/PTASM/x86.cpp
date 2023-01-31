#include <stdlib.h>

#include "x86.hpp"
#include "ptasm.hpp"

void X86Asm::assemble(AsmFile *file) {
    elf = new Elf64File(file->getName() + ".o");
    elf->addFunctionSymbol("_start", 0, true, true);
    
    for (size_t i = 0; i<file->getFunctionCount(); i++) {
        assembleFunction(file->getFunctionAt(i));
    }
    
    elf->write();
}

void X86Asm::link() {
    std::string cmd = "ld ";
    cmd += "/usr/local/lib/tinylang/ti_start.o ";
    //cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += "./output.o -o output ";
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    cmd += "-ltinylang -lc";
    system(cmd.c_str());
}

void X86Asm::assembleFunction(AsmFunction *func) {
    elf->addFunctionSymbol(func->getName(), lc, true, false);
    
    //
    // Setup the stack
    //
    // push rbp
    elf->addCode8(0x55);
    ++lc;
    
    // mov rbp, rsp
    elf->addCode8(0x48);
    elf->addCode8(0x89);
    elf->addCode8(0xE5);
    lc += 3;
    
    // sub rsp, 16
    elf->addCode8(0x48);
    elf->addCode8(0x83);
    elf->addCode8(0xEC);
    elf->addCode8(0x10);    // = 16 -> stack size. TODO: Change
    lc += 4;
    
    // Assemble the block
    for (size_t i = 0; i<func->getBlockCount(); i++) {
        assembleBlock(func->getBlockAt(i));
    }
    
    //TMP
    // mov eax, 60 = b8 3c 00 00 00
    /*elf->addCode8(0xB8);
    elf->addCode32(60);
    
    // mov edi, 5 = bf 05 00 00 00
    elf->addCode8(0xBF);
    elf->addCode32(5);
    
    // syscall = 0f 05
    elf->addCode8(0x0F);
    elf->addCode8(0x05);*/
}

void X86Asm::assembleBlock(AsmBlock *block) {
    for (size_t i = 0; i<block->getInstrCount(); i++) {
        AsmInstruction *instr = block->getInstrAt(i);
        switch (instr->getType()) {
            // Math
            case V_AsmType::Add: break;
            case V_AsmType::Sub: break;
            case V_AsmType::Mul: break;
            case V_AsmType::UDiv: break;
            case V_AsmType::SDiv: break;
            case V_AsmType::URem: break;
            case V_AsmType::SRem: break;
            
            // Load and store
            case V_AsmType::Ld: break;
            case V_AsmType::St: break;
            
            // Branch
            case V_AsmType::Br: break;
            case V_AsmType::Beq: break;
            case V_AsmType::Bne: break;
            case V_AsmType::Bg: break;
            case V_AsmType::Bl: break;
            case V_AsmType::Bge: break;
            case V_AsmType::Ble: break;
            
            //
            // Control flow
            //
            case V_AsmType::Call: break;
            
            // Assemble a return statement
            case V_AsmType::Ret: {
                if (instr->getSrc1Operand()) {
                    AsmOperand *src = instr->getSrc1Operand();
                    
                    switch (src->getType()) {
                        case V_AsmType::Reg: break;
                        
                        case V_AsmType::Int: {
                            elf->addCode8(0xB8);
                        
                            AsmInt *i = static_cast<AsmInt *>(src);
                            elf->addCode32(i->getValue());
                            lc += 5;
                        } break;
                        
                        default: {}
                    }
                }
                
                elf->addCode8(0xC9);        // leave
                elf->addCode8(0xC3);        // ret
                lc += 2;
            } break;
        
            default: {}
        }
    }
}

