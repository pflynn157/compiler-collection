//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <cstdlib>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <java/JavaCompiler.hpp>

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }
    
    // Parse the command line
    std::string input = "";
    bool print_ast = false;
    bool run_javap = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--ast") {
            print_ast = true;
        } else if (arg == "--javap") {
            run_javap = true;
        } else if (arg[0] == '-') {
            std::cerr << "Invalid option: " << arg << std::endl;
            return 1;
        } else {
            input = arg;
        }
    }
    
    // Now parse the input
    auto parser = std::make_unique<Parser>(input, true);
    if (!parser->parse()) {
        return 1;
    }
    
    auto tree = parser->getTree();
    
    if (print_ast) {
        tree->print();
        return 0;
    }
    
    // Finally, run the java compiler
    std::string className = GetClassName(input);
    std::cout << "Output: " << className << ".class" << std::endl;
    
    auto compiler = std::make_unique<Compiler>(className);
    compiler->Build(tree);
    compiler->Write();
    
    if (run_javap) {
        className += ".class";
        std::string cmd = "javap -verbose " + className;
        system(cmd.c_str());
    }

    return 0;
}

