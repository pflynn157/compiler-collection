#include <iostream>
#include <string>
#include <memory>

#include <lex/lex.hpp>
#include <parser/parser.hpp>
#include <midend/midend.hpp>
#include <llvm/Compiler.hpp>

void test_lex(std::string input) {
    std::unique_ptr<Lex> lex = std::make_unique<Lex>(input);
    token t = lex->get_next();
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
    cmd += "-lc";
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
    std::string input = "../first.sia";
    test_lex(input);
    std::cout << "----------------------------" << std::endl;
    
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(input);
    parser->parse();
    
    auto tree = parser->getTree();
    tree->print();
    
    std::cout << "----------------------------" << std::endl;
     
    std::unique_ptr<SiaMidend> midend = std::make_unique<SiaMidend>(tree);
    midend->run();
    tree = midend->tree;
    
    tree->print();
    
    std::cout << "----------------------------" << std::endl;
    
    CFlags flags;
    flags.name = "first";
    
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(tree, flags);
    compiler->compile();
    compiler->writeAssembly();
    
    compiler->debug();
    
    assemble(flags);
    link(flags);
    
    return 0;
}

