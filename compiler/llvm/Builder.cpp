//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace llvm;
using namespace llvm::sys;

#include "Compiler.hpp"

void Compiler::writeAssembly() {
    std::string triple = "";

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmParser();
    LLVMInitializeX86AsmPrinter();
    
    triple = sys::getDefaultTargetTriple();
    mod->setTargetTriple(triple);
    
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
    
    TargetOptions options;
    auto RM = Optional<Reloc::Model>();
    auto machine = target->createTargetMachine(triple, CPU, features, options, RM);
    mod->setDataLayout(machine->createDataLayout());
    
    // Write it out
    std::string outputPath = "/tmp/" + cflags.name + ".asm";
    
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

