#include <iostream>
#include <string>
#include <memory>

#include <lex/lex.hpp>
#include <parser/parser.hpp>
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

int main(int argc, char **argv) {
    std::string input = "../first.sia";
    test_lex(input);
    std::cout << "----------------------------" << std::endl;
    
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(input);
    parser->parse();
    
    auto tree = parser->getTree();
    tree->print();
    
    std::cout << "----------------------------" << std::endl;
    
    CFlags flags;
    flags.name = "first";
    
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(tree, flags);
    compiler->compile();
    compiler->writeAssembly();
    
    compiler->debug();
    
    return 0;
}

