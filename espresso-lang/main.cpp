//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>

#include <JavaCompiler.hpp>

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }
    
    // Other flags
    std::string input = "";
    bool testLex = false;
    bool printAst = false;
    bool runJavaP = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--test-lex") {
            testLex = true;
        } else if (arg == "--ast") {
            printAst = true;
        } else if (arg == "--javap") {
            runJavaP = true;
        } else if (arg[0] == '-') {
            std::cerr << "Invalid option: " << arg << std::endl;
            return 1;
        } else {
            input = arg;
        }
    }
    
    std::unique_ptr<Parser> frontend = std::make_unique<Parser>(input);
    std::shared_ptr<AstTree> tree;
    
    if (testLex) {
        frontend->debugScanner();
        return 0;
    }
    
    if (!frontend->parse()) {
        return 1;
    }
    
    tree = frontend->getTree();
    
    if (printAst) {
        tree->print();
        return 0;
    }

    //test
    std::string className = GetClassName(input);
    std::cout << "Output: " << className << ".class" << std::endl;
    
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(className);
    compiler->Build(tree);
    compiler->Write();
    
    if (runJavaP) {
        className += ".class";
        std::string cmd = "javap -verbose " + className;
        system(cmd.c_str());
    }
    
    return 0;
}

