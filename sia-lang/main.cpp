#include <iostream>
#include <string>
#include <memory>

#include <lex/lex.hpp>

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
    
    return 0;
}

