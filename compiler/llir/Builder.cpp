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

// Assemble the file
// TODO: This needs to be done properly. System() != proper. I was lazy
void Compiler::assemble(bool use_as) {
    if (use_as) {
        printf("Using built-in assembler...\n");
        std::string cmd = std::string(AS_LOCATION) + "/asx86 ";
        cmd += "/tmp/" + cflags.name + ".asm /tmp/" + cflags.name + ".o";
        system(cmd.c_str());
    } else {
        std::string cmd = "as /tmp/" + cflags.name + ".asm -o /tmp/" + cflags.name + ".o";
        system(cmd.c_str());
    }
}

// Link
// TODO: Same as above...
#ifdef DEV_LINK_MODE

#ifndef LINK_LOCATION
#define LINK_LOCATION = "."
#endif

void Compiler::link() {
    std::string cmd = "ld ";
    cmd += std::string(LINK_LOCATION) + "/amd64_start.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -L" + std::string(LINK_LOCATION) + "/corelib -lcorelib ";
    system(cmd.c_str());
    //printf("LINK: %s\n", cmd.c_str());
}

#else

void Compiler::link() {
    /*std::string cmd = "ld ";
    cmd += "/usr/local/lib/tinylang/ti_start.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    cmd += "-ltinylang -lc";
    system(cmd.c_str());*/
}

#endif

