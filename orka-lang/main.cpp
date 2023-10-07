//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <cstdlib>

#include <preproc/Preproc.hpp>
#include <parser/Parser.hpp>
#include <ast/ast.hpp>

#include <llvm/Compiler.hpp>

bool isError = false;

std::shared_ptr<AstTree> getAstTree(std::string input, bool testLex, bool printAst, bool emitDot) {
    std::unique_ptr<Parser> frontend = std::make_unique<Parser>(input);
    std::shared_ptr<AstTree> tree;
    
    if (testLex) {
        frontend->debugScanner();
        isError = false;
        return nullptr;
    }
    
    if (!frontend->parse()) {
        isError = true;
        return nullptr;
    }
    
    tree = frontend->getTree();
    
    if (printAst) {
        tree->print();
        return nullptr;
    }
    
    if (emitDot) {
        tree->dot();
        return nullptr;
    }
    
    return tree;
}

void assemble(CFlags cflags) {
    std::string cmd = "as /tmp/" + cflags.name + ".asm -o /tmp/" + cflags.name + ".o";
    system(cmd.c_str());
}

#ifdef DEV_LINK_MODE

#ifndef LINK_CORELIB_LOCATION
#define LINK_CORELIB_LOCATION = "."
#endif

#ifndef LINK_STDLIB_LOCATION
#define LINK_STDLIB_LOCATION = "."
#endif

void link(CFlags cflags) {
    std::string cmd = "ld ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -L" + std::string(LINK_STDLIB_LOCATION) + " -lstdlib ";
    cmd += " -L" + std::string(LINK_CORELIB_LOCATION) + " -lcorelib ";
    //cmd += " -L" + std::string(LINK_STDLIB_LOCATION) + " -lstdlib ";
    system(cmd.c_str());
    //printf("LINK: %s\n", cmd.c_str());
}

#else

void link(CFlags cflags) {
    /*std::string cmd = "ld ";
    cmd += "/usr/local/lib/tinylang/ti_start.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    cmd += "-ltinylang -lc";
    system(cmd.c_str());*/
}

#endif

int compileLLVM(std::shared_ptr<AstTree> tree, CFlags flags, bool printLLVM, bool emitLLVM) {
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(tree, flags);
    compiler->compile();
        
    if (printLLVM) {
        compiler->debug();
        return 0;
    }
    
    if (emitLLVM) {
        std::string output = flags.name;
        if (output == "a.out") {
            output = "./out.ll";
        }
            
        compiler->emitLLVM(output);
        return 0;
    }
        
    compiler->writeAssembly();
    assemble(flags);
    link(flags);
    
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }
    
    // Compiler (codegen) flags
    CFlags flags;
    flags.name = "a.out";
    
    // Other flags
    std::string input = "";
    bool emitPreproc = false;
    bool testLex = false;
    bool printAst = false;
    bool emitDot = false;
    bool printLLVM = false;
    bool emitLLVM = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-E") {
            emitPreproc = true;
        } else if (arg == "--test-lex") {
            testLex = true;
        } else if (arg == "--ast") {
            printAst = true;
        } else if (arg == "--dot") {
            emitDot = true;
        } else if (arg == "--llvm") {
            printLLVM = true;
        } else if (arg == "--emit-llvm") {
            emitLLVM = true;
        } else if (arg == "-o") {
            flags.name = argv[i+1];
            i += 1;
        } else if (arg[0] == '-') {
            std::cerr << "Invalid option: " << arg << std::endl;
            return 1;
        } else {
            input = arg;
        }
    }
    
    std::string newInput = preprocessFile(input, emitPreproc);
    if (newInput == "") {
        return 1;
    }
    
    std::shared_ptr<AstTree> tree = getAstTree(newInput, testLex, printAst, emitDot);
    if (tree == nullptr) {
        if (isError) return 1;
        return 0;
    }

    // Compile
    return compileLLVM(tree, flags, printLLVM, emitLLVM);
}

