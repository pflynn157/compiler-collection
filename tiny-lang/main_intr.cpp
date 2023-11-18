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
#include <intr/interpreter.hpp>

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }
    
    // Parse the command line
    std::string input = "";
    bool print_ast = false;
    
    for (int i = 1; i<argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--ast") {
            print_ast = true;
        } else if (arg[0] == '-') {
            std::cerr << "Invalid option: " << arg << std::endl;
            return 1;
        } else {
            input = arg;
        }
    }
    
    // Now parse the input
    auto parser = std::make_unique<Parser>(input);
    if (!parser->parse()) {
        return 1;
    }
    
    auto tree = parser->getTree();
    
    if (print_ast) {
        tree->print();
        return 0;
    }
    
    auto intr = std::make_unique<AstInterpreter>(tree);
    int code = intr->run();

    return code;
}

