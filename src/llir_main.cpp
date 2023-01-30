//
// Copyright 2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <cstdio>

#include <preproc/Preproc.hpp>
#include <parser/Parser.hpp>
#include <ast/ast.hpp>

#include <compiler/LLIR/Compiler.hpp>

bool isError = false;

// TODO: I'm not sure actually if the lex testing actually works
//
AstTree *getAstTree(std::string input, bool testLex, bool printAst) {
    Parser *frontend = new Parser(input);
    AstTree *tree;
    
    if (testLex) {
        frontend->debugScanner();
        isError = false;
        return nullptr;
    }
    
    if (!frontend->parse()) {
        delete frontend;
        isError = true;
        return nullptr;
    }
    
    tree = frontend->getTree();
    
    delete frontend;
    remove(input.c_str());
    
    if (printAst) {
        tree->print();
        return nullptr;
    }
    
    return tree;
}

int compileLLIR(AstTree *tree, CFlags flags, bool printLLVM, bool printLLIR2) {
    Compiler *compiler = new Compiler(tree, flags);
    compiler->compile();
        
    if (printLLVM) {
        compiler->debug();
        return 0;
    }
        
    compiler->writeAssembly(printLLIR2);
    compiler->assemble();
    compiler->link();
    
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
    bool printLLVM = false;
    bool printLLIR2 = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-E") {
            emitPreproc = true;
        } else if (arg == "--test-lex") {
            testLex = true;
        } else if (arg == "--ast") {
            printAst = true;
        } else if (arg == "--llir") {
            printLLVM = true;
        } else if (arg == "--llir2") {
            printLLIR2 = true;
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
    
    AstTree *tree = getAstTree(newInput, testLex, printAst);
    if (tree == nullptr) {
        if (isError) return 1;
        return 0;
    }

    // Compile
    return compileLLIR(tree, flags, printLLVM, printLLIR2);
}

