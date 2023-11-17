#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <functional>

#include <lex/lex.hpp>
#include <parser/parser.hpp>
#include <midend/midend.hpp>
#include <midend/parallel_midend.hpp>
#include <llvm/Compiler.hpp>

void test_lex(std::string input) {
    std::unique_ptr<Lex> lex = std::make_unique<Lex>(input);
    int t = lex->get_next();
    while (t != t_eof) {
        lex->debug_token(t);
        t = lex->get_next();
    }
    lex->debug_token(t);
}

void assemble(CFlags cflags) {
    std::string cmd = "as /tmp/" + cflags.name + ".asm -o /tmp/" + cflags.name + ".o";
    system(cmd.c_str());
}

#ifdef DEV_LINK_MODE

#ifndef LINK_LOCATION
#define LINK_LOCATION = "."
#endif

void link(CFlags cflags) {
    std::string cmd = "ld ";
    cmd += "/usr/lib/x86_64-linux-gnu/crt1.o ";
    cmd += "/usr/lib/x86_64-linux-gnu/crti.o ";
    cmd += "/usr/lib/x86_64-linux-gnu/crtn.o ";
    cmd += "/tmp/" + cflags.name + ".o -o " + cflags.name;
    cmd += " -dynamic-linker /lib64/ld-linux-x86-64.so.2 ";
    cmd += "-lc -lomp5";
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

int main(int argc, char **argv) {
    std::string input = "../test/first.sia";
    std::string name = std::filesystem::path(input).stem();
    
    std::function<void(std::string)> f_test_lex = test_lex;
    f_test_lex(input);
    
    //test_lex(input);
    std::cout << "----------------------------" << std::endl;
    
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(input);
    parser->parse();
    
    auto tree = parser->getTree();
    tree->print();
    
    std::cout << "----------------------------" << std::endl;
    
    // Run the parallel processing midend
    auto midend1 = std::make_unique<ParallelMidend>(tree);
    midend1->run();
    tree = midend1->tree;
     
    // Run the general midend
    auto midend2 = std::make_unique<Midend>(tree);
    midend2->run();
    tree = midend2->tree;
    
    tree->print();
    
    std::cout << "----------------------------" << std::endl;
    
    CFlags flags;
    flags.name = name;
    std::cout << "FILENAME: " << name << std::endl;
    
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(tree, flags);
    compiler->compile();
    compiler->writeAssembly();
    
    compiler->debug();
    
    assemble(flags);
    link(flags);
    
    return 0;
}

