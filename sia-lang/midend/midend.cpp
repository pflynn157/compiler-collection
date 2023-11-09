#include <memory>
#include <iostream>

#include "midend.hpp"

void Midend::process_block_statement(std::shared_ptr<AstBlockStmt> stmt, std::shared_ptr<AstBlock> block) {
    std::cout << "MIDEND" << std::endl;
}

