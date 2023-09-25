//
// Copyright 2022 Patrick Flynn
// This file is part of the LLIR framework.
// LLIR is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <parser.hpp>
#include <amd64/amd64.hpp>

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cout << "Error: No input file specified." << std::endl;
        return 0;
    }
    
    std::string input = "";
    std::string output = "a.out";
    bool print = false;
    bool print2 = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg == "--llir") {
            print = true;
        } else if (arg == "--llir2") {
            print2 = true;
        } else if (arg == "-o") {
            output = std::string(argv[i+1]);
            ++i;
        } else if (arg[0] == '-') {
            std::cerr << "Error: Invalid argument." << std::endl;
            return 1;
        } else {
            input = arg;
        }
    }
    
    Parser *parser = new Parser(input, output);
    parser->parse();
    if (print) parser->print();
    
    // Generate assembly and compile
    std::string asmFile = "/tmp/" + output + ".s";
    std::string objFile = "/tmp/" + output + ".o";
    std::string command1 = "as " + asmFile + " -o " + objFile;
    std::string command2 = "ld /usr/lib/x86_64-linux-gnu/crt1.o ";
        command2 += "/usr/lib/x86_64-linux-gnu/crti.o /usr/lib/x86_64-linux-gnu/crtn.o ";
        command2 += objFile + " -o " + output;
        command2 += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc";
    
    Module *mod = parser->getModule();
    mod->transform();
    if (print2) mod->print();
    
    LLIR::Amd64Writer *writer = new LLIR::Amd64Writer(mod);
    writer->compile();
    writer->writeToFile(asmFile);
    
    system(command1.c_str());
    system(command2.c_str());
    
    // Clean up
    delete parser;
    delete writer;
    
    return 0;
}
