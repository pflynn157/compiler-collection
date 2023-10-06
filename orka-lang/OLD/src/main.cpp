//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <cstdio>

#include <preproc/Preproc.hpp>
#include <parser/Parser.hpp>
#include <ast.hpp>

#include <LLVM/Compiler.hpp>
#include <LLIR/LLIRCompiler.hpp>

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

int compileLLVM(AstTree *tree, CFlags flags, bool printLLVM, bool emitLLVM, bool emitNVPTX) {
    Compiler *compiler = new Compiler(tree, flags);
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
    
    if (!emitNVPTX) {
        compiler->assemble();
        compiler->link();
    }
    
    return 0;
}

int compileLLIR(AstTree *tree, std::string outputName) {
    LLIRCompiler *compiler = new LLIRCompiler(tree, outputName);
    compiler->compile();
    compiler->debug();
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
    flags.nvptx = false;
    
    // Other flags
    std::string input = "";
    bool testLex = false;
    bool printAst = false;
    bool printLLVM = false;
    bool emitLLVM = false;
    bool emitNVPTX = false;
    bool useLLVM = true;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--test-lex") {
            testLex = true;
        } else if (arg == "--ast") {
            printAst = true;
        } else if (arg == "--llvm") {
            printLLVM = true;
        } else if (arg == "--emit-llvm") {
            emitLLVM = true;
        } else if (arg == "--emit-nvptx") {
            emitNVPTX = true;
            flags.nvptx = true;
        } else if (arg == "--host") {
            useLLVM = false;
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
    
    std::string newInput = preprocessFile(input);
    if (newInput == "") {
        return 1;
    }
    
    AstTree *tree = getAstTree(newInput, testLex, printAst);
    if (tree == nullptr) {
        if (isError) return 1;
        return 0;
    }

    //test
    if (useLLVM) {
        return compileLLVM(tree, flags, printLLVM, emitLLVM, emitNVPTX);
    } else {
        return compileLLIR(tree, "output1");
    }
    
    return 0;
}

