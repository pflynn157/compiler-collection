#include <string>

#include <ast/ast_builder.hpp>
#include <ast/ast.hpp>

namespace AstBuilder {

//
// The builders for data types
//
std::shared_ptr<AstDataType> buildVoidType() {
    return std::make_shared<AstDataType>(V_AstType::Void);
}

std::shared_ptr<AstDataType> buildBoolType() {
    return std::make_shared<AstDataType>(V_AstType::Bool);
}

std::shared_ptr<AstDataType> buildCharType() {
    return std::make_shared<AstDataType>(V_AstType::Char);
}

std::shared_ptr<AstDataType> buildInt8Type(bool isUnsigned) {
    return std::make_shared<AstDataType>(V_AstType::Int8, isUnsigned);
}

std::shared_ptr<AstDataType> buildInt16Type(bool isUnsigned) {
    return std::make_shared<AstDataType>(V_AstType::Int16, isUnsigned);
}

std::shared_ptr<AstDataType> buildInt32Type(bool isUnsigned) {
    return std::make_shared<AstDataType>(V_AstType::Int32, isUnsigned);
}

std::shared_ptr<AstDataType> buildInt64Type(bool isUnsigned) {
    return std::make_shared<AstDataType>(V_AstType::Int64, isUnsigned);
}

std::shared_ptr<AstDataType> buildStringType() {
    return std::make_shared<AstDataType>(V_AstType::String);
}

std::shared_ptr<AstPointerType> buildPointerType(std::shared_ptr<AstDataType> base) {
    return std::make_shared<AstPointerType>(base);
}

std::shared_ptr<AstStructType> buildStructType(std::string name) {
    return std::make_shared<AstStructType>(name);
}

std::shared_ptr<AstObjectType> buildObjectType(std::string name) {
    return std::make_shared<AstObjectType>(name);
}

} // End AstBuilder

