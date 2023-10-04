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

//
// AstDataType
//
AstDataType::AstDataType(V_AstType type) : AstNode(type) {}
AstDataType::AstDataType(V_AstType type, bool _isUnsigned) : AstNode(type) {
    this->is_unsigned = _isUnsigned;
}

//
// AstPointerType
//
AstPointerType::AstPointerType(std::shared_ptr<AstDataType> base_type) : AstDataType(V_AstType::Ptr) {
    this->base_type = base_type;
}

//
// AstStructType
//
AstStructType::AstStructType(std::string name) : AstDataType(V_AstType::Struct) {
    this->name = name;
}

//
// AstObjectType
//
AstObjectType::AstObjectType() : AstDataType(V_AstType::Object) {}

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

void AstStruct::addItem(Var var, std::shared_ptr<AstExpression> default_expression) {
    items.push_back(var);
    default_expressions[var.name] = default_expression;
    
    switch (var.type->type) {
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

//
// AstTree
//
AstTree::AstTree(std::string file) {
    this-> file = file;
    this->block = std::make_shared<AstBlock>();
}

AstTree::~AstTree() {}

bool AstTree::hasStruct(std::string name) {
    for (auto const &s : structs) {
        if (s->name == name) return true;
    }
    return false;
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
    // Scoped variables
    auto map = parent->getSymbolTable();
    for (auto const &element : map) {
        symbolTable[element.first] = element.second;
        vars.push_back(element.first);
    }
    
    // Global constants
    for (auto const &element : parent->globalConsts) {
        globalConsts[element.first] = element.second;
    }
    
    // Scoped constants
    for (auto const &element : parent->localConsts) {
        localConsts[element.first] = element.second;
    }
    
    // Functions
    auto parent_funcs = parent->funcs;
    for (auto func : parent_funcs) {
        this->funcs.push_back(func);
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

int AstBlock::isConstant(std::string name) {
    if (globalConsts.find(name) != globalConsts.end()) {
        return 1;
    }
    
    if (localConsts.find(name) != localConsts.end()) {
        return 2;
    }
    
    return 0;
}

bool AstBlock::isFunc(std::string name) {
    if (std::find(funcs.begin(), funcs.end(), name) != funcs.end()) {
        return true;
    }
    return false;
}

//
// AstStatement
//
AstStatement::AstStatement() : AstNode(V_AstType::None) {}
AstStatement::AstStatement(V_AstType type) : AstNode(type) {}

bool AstStatement::hasExpression() {
    if (expression) return true;
    return false;
}

