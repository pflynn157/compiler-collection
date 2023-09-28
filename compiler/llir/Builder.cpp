//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//

#include "Compiler.hpp"
#include <amd64/amd64.hpp>

void Compiler::writeAssembly(bool printTransform) {
    // Run the transform pass so it can be printed
    mod->transform();
    if (printTransform) {
        mod->print();
        return;
    }

    // Write it out
    std::string outputPath = "/tmp/" + cflags.name + ".asm";
    
    LLIR::Amd64Writer *writer = new LLIR::Amd64Writer(mod);
    writer->compile();
    writer->writeToFile(outputPath);
}

