#include <algorithm>
#include <vector>

#include <ast/ast.hpp>

//
// AstNode
//
AstNode::AstNode() {}
AstNode::AstNode(V_AstType type) {
    this->type = type;
}

V_AstType AstNode::getType() { return type; }

//
// AstDataType
//
AstDataType::AstDataType(V_AstType type) : AstNode(type) {}
AstDataType::AstDataType(V_AstType type, bool _isUnsigned) : AstNode(type) {
    this->_isUnsigned = _isUnsigned;
}

void AstDataType::setUnsigned(bool _isUnsigned) { this->_isUnsigned = _isUnsigned; }

bool AstDataType::isUnsigned() { return _isUnsigned; }

//
// AstPointerType
//
AstPointerType::AstPointerType(std::shared_ptr<AstDataType> baseType) : AstDataType(V_AstType::Ptr) {
    this->baseType = baseType;
}

std::shared_ptr<AstDataType> AstPointerType::getBaseType() { return baseType; }

//
// AstStructType
//
AstStructType::AstStructType(std::string name) : AstDataType(V_AstType::Struct) {
    this->name = name;
}

std::string AstStructType::getName() { return name; }

//
// Var
//
Var::Var() {}
Var::Var(std::shared_ptr<AstDataType> type, std::string name) {
    this->type = type;
    this->name = name;
}

//
// AstStruct
//
AstStruct::AstStruct(std::string name) : AstNode(V_AstType::StructDef) {
    this->name = name;
}

void AstStruct::addItem(Var var, std::shared_ptr<AstExpression> defaultExpression) {
    items.push_back(var);
    defaultExpressions[var.name] = defaultExpression;
    
    switch (var.type->getType()) {
        case V_AstType::Char:
        case V_AstType::Int8: size += 1; break;
        case V_AstType::Int16: size += 2; break;
        case V_AstType::Bool:
        case V_AstType::Int32: size += 4; break;
        case V_AstType::String:
        case V_AstType::Ptr:
        case V_AstType::Struct:
        case V_AstType::Int64: size += 8; break;
        
        default: {}
    }
}

std::string AstStruct::getName() { return name; }
std::vector<Var> AstStruct::getItems() { return items; }
int AstStruct::getSize() { return size; }

std::shared_ptr<AstExpression> AstStruct::getDefaultExpression(std::string name) {
    return defaultExpressions[name];
}

//
// AstTree
//
AstTree::AstTree(std::string file) { this-> file = file; }
AstTree::~AstTree() {}

std::string AstTree::getFile() { return file; }

std::vector<std::shared_ptr<AstGlobalStatement>> AstTree::getGlobalStatements() {
    return global_statements;
}

std::vector<std::shared_ptr<AstStruct>> AstTree::getStructs() {
    return structs;
}

bool AstTree::hasStruct(std::string name) {
    for (auto const &s : structs) {
        if (s->getName() == name) return true;
    }
    return false;
}

void AstTree::addGlobalStatement(std::shared_ptr<AstGlobalStatement> stmt) {
    global_statements.push_back(stmt);
}

void AstTree::addStruct(std::shared_ptr<AstStruct> s) {
    structs.push_back(s);
}

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

void AstBlock::addSymbol(std::string name, std::shared_ptr<AstDataType> dataType) {
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

std::map<std::string, std::shared_ptr<AstDataType>> AstBlock::getSymbolTable() {
    return symbolTable;
}

std::shared_ptr<AstDataType> AstBlock::getDataType(std::string name) {
    return symbolTable[name];
}

bool AstBlock::isVar(std::string name) {
    if (std::find(vars.begin(), vars.end(), name) != vars.end()) {
        return true;
    }
    return false;
}

//
// AstStatement
//
AstStatement::AstStatement() : AstNode(V_AstType::None) {}
AstStatement::AstStatement(V_AstType type) : AstNode(type) {}

void AstStatement::setExpression(std::shared_ptr<AstExpression> expr) { this->expr = expr; }
std::shared_ptr<AstExpression> AstStatement::getExpression() { return expr; }
bool AstStatement::hasExpression() {
    if (expr) return true;
    return false;
}

