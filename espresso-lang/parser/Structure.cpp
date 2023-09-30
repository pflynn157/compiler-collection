//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
// Structure.cpp
// Handles parsing for enums and structs
#include <map>

#include <parser/Parser.hpp>
#include <ast.hpp>

// Parses and builds an enumeration
bool Parser::buildEnum() {
    Token token = scanner->getNext();
    std::string name = token.id_val;
    
    if (token.type != Id) {
        syntax->addError(scanner->getLine(), "Expected enum name.");
        return false;
    }
    
    // See if we have a type for the enum. Default is int
    token = scanner->getNext();
    DataType dataType = DataType::Int32;
    bool useDefault = false;
    
    switch (token.type) {
        case Bool: dataType = DataType::Bool; break;
        case Char: dataType = DataType::Char; break;
        case Byte: dataType = DataType::Byte; break;
        case UByte: dataType = DataType::UByte; break;
        case Short: dataType = DataType::Short; break;
        case UShort: dataType = DataType::UShort; break;
        case Int: dataType = DataType::Int32; break;
        case UInt: dataType = DataType::UInt32; break;
        case Int64: dataType = DataType::Int64; break;
        case UInt64: dataType = DataType::UInt64; break;
        case Str: dataType = DataType::String; break;
        
        case Is: useDefault = true; break;
        
        default: {
            syntax->addError(scanner->getLine(), "Unknown token in enum declaration");
            return false;
        }
    }
    
    // Syntax check
    if (!useDefault) {
        token = scanner->getNext();
        if (token.type != Is) {
            syntax->addError(scanner->getLine(), "Expected \"is\"");
            return false;
        }
    }
    
    // Loop and get all the values
    std::map<std::string, AstExpression *> values;
    int index = 0;
    
    while (token.type != End && token.type != Eof) {
        token = scanner->getNext();
        std::string valName = token.id_val;
        
        if (token.type != Id) {
            syntax->addError(scanner->getLine(), "Expected enum value.");
            token.print();
            return false;
        }
        
        token = scanner->getNext();
        AstExpression *value = nullptr;
        
        if (token.type == Assign) {
        
        } else if (token.type != Comma && token.type != End) {
            syntax->addError(scanner->getLine(), "Unknown token in enum.");
            token.print();
            return false;
        }
        
        if (value == nullptr) {
            value = checkExpression(new AstInt(index), dataType);
            ++index;
        }
        
        values[valName] = value;
    }
    
    // Put it all together
    EnumDec theEnum;
    theEnum.name = name;
    theEnum.type = dataType;
    theEnum.values = values;
    enums[name] = theEnum;
    
    return true;
}
