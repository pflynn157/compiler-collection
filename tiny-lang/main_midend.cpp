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

#include <preproc/Preproc.hpp>
#include <parser/Parser.hpp>
#include <ast/ast.hpp>

#include <midend/ast_midend.hpp>
#include <midend/test_midend.hpp>

int main(int argc, char **argv) {
    if (argc == 1) {
        std::cerr << "Error: No input file specified." << std::endl;
        return 1;
    }
    
    std::string input = argv[1];
    
    std::unique_ptr<Parser> frontend = std::make_unique<Parser>(input);
    frontend->parse();
    std::shared_ptr<AstTree> tree = frontend->getTree();
    
    tree->print();
    
    std::cout << "------------------------------------" << std::endl;
    
    //---------------------------------------
    std::unique_ptr<TestMidend> midend = std::make_unique<TestMidend>(tree);
    midend->run();
    
    return 0;
}

