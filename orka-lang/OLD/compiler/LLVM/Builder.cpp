//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace llvm;
using namespace llvm::sys;

#include <LLVM/Compiler.hpp>

void Compiler::writeAssembly() {
    std::string triple = "";

    if (cflags.nvptx) {
        LLVMInitializeNVPTXTargetInfo();
        LLVMInitializeNVPTXTarget();
        LLVMInitializeNVPTXTargetMC();
        //LLVMInitializeNVPTXAsmParser();
        LLVMInitializeNVPTXAsmPrinter();
        
        triple = "nvptx64-nvidia-cuda";
        mod->setTargetTriple(triple);
        
        mod->setDataLayout("p:64:64:64");
    } else {
        LLVMInitializeX86TargetInfo();
        LLVMInitializeX86Target();
        LLVMInitializeX86TargetMC();
        LLVMInitializeX86AsmParser();
        LLVMInitializeX86AsmPrinter();
        
        triple = sys::getDefaultTargetTriple();
        mod->setTargetTriple(triple);
    }
    
    std::string error;
    auto target = TargetRegistry::lookupTarget(triple, error);
    
    // Check for any errors with the target triple
    if (!target) {
        errs() << error;
        return;
    }
    
    // CPU and features
    auto CPU = "generic";
    auto features = "";
    
    if (cflags.nvptx) CPU = "";
    
    TargetOptions options;
    auto RM = Optional<Reloc::Model>();
    auto machine = target->createTargetMachine(triple, CPU, features, options, RM);
    mod->setDataLayout(machine->createDataLayout());
    
    // Write it out
    std::string outputPath = cflags.name;
    if (cflags.nvptx) outputPath += ".ptx";
    else outputPath = "/tmp/" + outputPath + ".asm";
    
    std::error_code errorCode;
    raw_fd_ostream writer(outputPath, errorCode, sys::fs::OF_None);
    
    if (errorCode) {
        errs() << "Unable to open file: " << errorCode.message();
        return;
    }
    
    legacy::PassManager pass;
    auto outputType = CGFT_AssemblyFile;
    
    if (machine->addPassesToEmitFile(pass, writer, nullptr, outputType)) {
        errs() << "Unable to write to file.";
        return;
    }
    
    pass.run(*mod);
    writer.flush();
}

// Assemble the file
// TODO: This needs to be done properly. System() != proper. I was lazy
void Compiler::assemble() {
    std::string cmd = "as /tmp/" + cflags.name + ".asm -o /tmp/" + cflags.name + ".o";
    system(cmd.c_str());
}

// Link
// TODO: Same as above...
// Also... We shouldn't be using GCC to link
void Compiler::link() {
    std::string cmd = "ld ";
#ifdef LINK_FEDORA
    //cmd += "/usr/lib64/crt1.o /usr/lib64/crti.o /usr/lib64/crtn.o ";
#else
    //cmd += "/usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o /usr/lib/x86_64-linux-gnu/crtn.o ";
#endif
    cmd += "/usr/local/lib/orka/occ_start.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    //cmd += "-lc";
    cmd += "-lorka -lorka_corelib";
    system(cmd.c_str());
}

