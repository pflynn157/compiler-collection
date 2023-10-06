#include <LLIR/X86Writer.hpp>

X86Writer::X86Writer(PASMFile *file) {
    this->file = file;
    outputPath = "/tmp/" + file->getName() + ".asm";
    writer = std::ofstream(outputPath);
}

void X86Writer::compile() {
    writer << ".intel_syntax noprefix" << std::endl;
    writer << ".data" << std::endl;
    
    writer << ".text" << std::endl;
    
    for (PASMInstruction *ln : file->getCode()) {
        writeInstruction(ln);
    }
}

void X86Writer::assemble() {
    std::string cmd = "as " + outputPath + " -o /tmp/" + file->getName() + ".o";
    system(cmd.c_str());
}

void X86Writer::link() {
    std::string cmd = "ld ";
    cmd += "/usr/local/lib/orka/occ_start.o ";
    cmd += "/tmp/" + file->getName() + ".o -o " + file->getName();
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    //cmd += "-lc";
    cmd += "-lorka -lorka_corelib";
    system(cmd.c_str());
}

void X86Writer::writeInstruction(PASMInstruction *line) {
    if (line->getOpType() == PASM::Label) {
        PASMLabel *label = static_cast<PASMLabel *>(line);
        writer << label->getName() << ":" << std::endl;
        return;
    } else if (line->getOpType() == PASM::Func) {
        PASMFunc *func = static_cast<PASMFunc *>(line);
        writer << ".global " << func->getName() << std::endl;
        writer << func->getName() << ":" << std::endl;
        return;
    }
    
    writer << "  ";
    
    switch (line->getOpType()) {
        case PASM::NOP: writer << "nop "; break;
        case PASM::LI: writer << "mov "; break;
        case PASM::RET: writer << "ret "; break;
        
        default: {}
    }
    
    if (line->getOperand1()) writeOperand(line->getOperand1());
    if (line->getOperand2()) {
        writer << ", ";
        writeOperand(line->getOperand2());
    }
    if (line->getOperand3()) {
        writer << ", ";
        writeOperand(line->getOperand3());
    }
    
    writer << std::endl;
}

void X86Writer::writeOperand(PASMOperand *op) {
    switch (op->getOpType()) {
        case PASM::Imm: {
            PASMImm *imm = static_cast<PASMImm *>(op);
            writer << imm->getValue();
        } break;
        
        case PASM::R8: {} break;
        case PASM::R16: {} break;
        
        case PASM::R32: {
            PASMReg32 *reg = static_cast<PASMReg32 *>(op);
            switch (reg->getRegNum()) {
                case 0: writer << "eax"; break;
                case 1: writer << "ebx"; break;
                case 2: writer << "ecx"; break;
                case 3: writer << "edx"; break;
                
                default: {}
            }
        } break;
        
        case PASM::R64: {} break;
        
        default: {}
    }
}

