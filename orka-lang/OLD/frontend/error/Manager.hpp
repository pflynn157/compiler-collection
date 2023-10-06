//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>

struct Error {
    int line;
    std::string message;
};

class ErrorManager {
public:
    void addError(int line, std::string message);
    void addWarning(int line, std::string message);
    bool errorsPresent();
    void printErrors();
    void printWarnings();
private:
    std::vector<Error> errors;
    std::vector<Error> warnings;
};

