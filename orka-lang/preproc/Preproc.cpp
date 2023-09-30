//
// Copyright 2021 Patrick Flynn
// This file is part of the Orka compiler.
// Orka is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <fstream>
#include <iostream>
#include <cstdio>

#include <lex/Lex.hpp>

std::string getInputPath(std::string input) {
    std::string name = "";
    for (int i = 0; i<input.length() - 3; i++) {
        char c = input.at(i);
        if (c == '/') name = "";
        else if (c == '.') continue;
        else name += c;
    }
    
    name += "_pre.ok";
    return name;
}

std::string preprocessFile(std::string input) {
    std::string newPath = "/tmp/" + getInputPath(input);
    Scanner *scanner = new Scanner(input);
    if (scanner->isError()) {
        return "";
    }
    
    std::ofstream writer(newPath);
    
    // Read until the end of the file
    Token token;
    while (!scanner->isEof() && token.type != Eof) {
        token = scanner->getNext();
        
        if (token.type != Import) {
            writer << scanner->getRawBuffer();
            continue;
        }
        
        // Build the include path
        token = scanner->getNext();
        std::string path = "";
        
        while (token.type != SemiColon) {
            switch (token.type) {
                case Id: path += token.id_val; break;
                case Dot: path += "/"; break;
                
                default: {
                    // TODO: Blow up
                }
            }
            
            token = scanner->getNext();
        }
        
        // Load the include path
        // TODO: We need better path support
        path = "/usr/local/include/orka/" + path + ".oh";
        std::string preprocInclude = preprocessFile(path);
        
        std::ifstream reader(preprocInclude.c_str());
        if (!reader.is_open()) {
            std::cerr << "Fatal: Unable to open preprocessed include" << std::endl;
            return "";
        }
        
        std::string line = "";
        while (std::getline(reader, line)) {
            writer << line;
        }
        
        reader.close();
        remove(preprocInclude.c_str());
        
        // Drop the buffer so we don't put the include line back in
        scanner->getRawBuffer();
    }
    
    delete scanner;
    return newPath;
}

