//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>

#include <preproc/Preproc.hpp>
#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <midend/midend.hpp>

#include <llir/Compiler.hpp>

bool isError = false;

std::shared_ptr<AstTree> getAstTree(std::string input, bool testLex, bool printAst1, bool printAst, bool emitDot) {
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
    
    if (printAst1) {
        tree->print();
        return nullptr;
    }
    
    std::unique_ptr<Midend> midend = std::make_unique<Midend>(tree);
    midend->run();
    tree = midend->tree;
    
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


// Assemble the file
// TODO: This needs to be done properly. System() != proper. I was lazy
void assemble(CFlags cflags, bool use_as) {
    if (use_as) {
        printf("Using built-in assembler...\n");
        std::string cmd = std::string(AS_LOCATION) + "/x86/asx86 ";
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

void link(CFlags cflags) {
    std::string cmd = "ld ";
    cmd += std::string(LINK_LOCATION) + "/amd64_start.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -L" + std::string(LINK_LOCATION) + "/corelib -lcorelib ";
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

int compileLLIR(std::shared_ptr<AstTree> tree, CFlags flags, bool printLLVM, bool printLLIR2, bool use_as) {
    Compiler *compiler = new Compiler(tree, flags);
    compiler->compile();
        
    if (printLLVM) {
        compiler->debug();
        return 0;
    }
        
    compiler->writeAssembly(printLLIR2);
    assemble(flags, use_as);
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
    bool printAst1 = false;
    bool printAst = false;
    bool emitDot = false;
    bool printLLVM = false;
    bool printLLIR2 = false;
    bool use_as = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-E") {
            emitPreproc = true;
        } else if (arg == "--test-lex") {
            testLex = true;
        } else if (arg == "--ast1") {
            printAst1 = true;
        } else if (arg == "--ast") {
            printAst = true;
        } else if (arg == "--dot") {
            emitDot = true;
        } else if (arg == "--llir") {
            printLLVM = true;
        } else if (arg == "--llir2") {
            printLLIR2 = true;
        } else if (arg == "--builtin-as" || arg == "-ba") {
            use_as = true;
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
    
    std::shared_ptr<AstTree> tree = getAstTree(newInput, testLex, printAst1, printAst, emitDot);
    if (tree == nullptr) {
        if (isError) return 1;
        return 0;
    }

    // Compile
    return compileLLIR(tree, flags, printLLVM, printLLIR2, use_as);
}

