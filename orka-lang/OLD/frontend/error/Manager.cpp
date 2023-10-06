//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>

#include <error/Manager.hpp>

// Adds a syntax error message
void ErrorManager::addError(int line, std::string message) {
    Error error;
    error.line = line;
    error.message = message;
    errors.push_back(error);
}

// Adds a syntax warning
void ErrorManager::addWarning(int line, std::string message) {
    Error error;
    error.line = line;
    error.message = message;
    warnings.push_back(error);
}

// Returns whether there are any errors
bool ErrorManager::errorsPresent() {
    if (errors.size() == 0) return false;
    return true;
}

// Prints any errors
void ErrorManager::printErrors() {
    for (Error err : errors) {
        std::cout << "[" << err.line << "] Syntax Error: " << err.message << std::endl;
    }
}

// Prints any warnings
void ErrorManager::printWarnings() {
    for (Error err : warnings) {
        std::cout << "[" << err.line << "] Warning: " << err.message << std::endl;
    }
}

