#include <iostream>
#include <memory>
#include <string>

#include <parser/parser.hpp>
#include <ast/ast.hpp>
#include <llvm/Compiler.hpp>
#include <lex/lex.hpp>

void test_scanner(std::string input) {
    Lex lex(input);
    token t = lex.get_next();
    while (t != t_eof) {
        lex.print(t);
        t = lex.get_next();
    }
    lex.print(t);
    
    std::cout << "----------------------------------------" << std::endl;
}

std::shared_ptr<AstTree> get_ast(std::string input) {
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(input);
    if (!parser->parse()) {
        return nullptr;
    }
    
    auto tree = parser->getTree();
    tree->print();
    
    return tree;
}

void assemble(std::string) {

}

void link(std::string name) {

}

void compile(std::shared_ptr<AstTree> tree, CFlags flags) {
    std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>(tree, flags);
    compiler->compile();
    compiler->writeAssembly();
    
    std::cout << "----------------------------------------" << std::endl;
    compiler->debug();
    
    assemble(flags.name);
    link(flags.name);
}

int main(int argc, char **argv) {
    std::string input = "../first.sal";
    test_scanner(input);
    
    std::shared_ptr<AstTree> tree = get_ast(input);
    
    CFlags flags;
    flags.name = "first";
    
    compile(tree, flags);
    
    return 0;
}

