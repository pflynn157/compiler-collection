#include <algorithm>
#include <vector>

#include <ast/ast.hpp>

//
// AstBlock
//
void AstBlock::addStatement(std::shared_ptr<AstStatement> stmt) {
    block.push_back(stmt);
}

void AstBlock::addStatements(std::vector<std::shared_ptr<AstStatement>> block) {
    this->block = block;
}

std::vector<std::shared_ptr<AstStatement>> AstBlock::getBlock() {
    return block;
}

size_t AstBlock::getBlockSize() {
    return block.size();
}

std::shared_ptr<AstStatement> AstBlock::getStatementAt(size_t i) {
    return block.at(i);
}

void AstBlock::removeAt(size_t pos) {
    block.erase(block.begin() + pos);
}

void AstBlock::insertAt(std::shared_ptr<AstStatement> stmt, size_t pos) {
    block.insert(block.begin() + pos, stmt);
}

void AstBlock::addSymbol(std::string name, AstDataType *dataType) {
    symbolTable[name] = dataType;
    vars.push_back(name);
}

void AstBlock::mergeSymbols(std::shared_ptr<AstBlock> parent) {
    auto map = parent->getSymbolTable();
    for (auto const &element : map) {
        symbolTable[element.first] = element.second;
        vars.push_back(element.first);
    }
}

std::map<std::string, AstDataType *> AstBlock::getSymbolTable() {
    return symbolTable;
}

AstDataType *AstBlock::getDataType(std::string name) {
    return symbolTable[name];
}

bool AstBlock::isVar(std::string name) {
    if (std::find(vars.begin(), vars.end(), name) != vars.end()) {
        return true;
    }
    return false;
}
