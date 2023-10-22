//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
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

