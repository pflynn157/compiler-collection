//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <fstream>
#include <iostream>
#include <cstdio>
#include <memory>

#include <lex/lex.hpp>

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

std::string preprocessFile(std::string input, bool print) {
    std::string newPath = "/tmp/" + getInputPath(input);
    std::unique_ptr<Scanner> scanner = std::make_unique<Scanner>(input);
    if (scanner->isError()) {
        return "";
    }
    
    std::ofstream writer(newPath);
    
    // Read until the end of the file
    Token token;
    while (!scanner->isEof() && token.type != t_eof) {
        token = scanner->getNext();
        
        if (token.type != t_import) {
            writer << scanner->getRawBuffer();
            continue;
        }
        
        // Build the include path
        token = scanner->getNext();
        std::string path = "";
        
        while (token.type != t_semicolon) {
            switch (token.type) {
                case t_id: path += token.id_val; break;
                case t_dot: path += "/"; break;
                
                default: {
                    // TODO: Blow up
                }
            }
            
            token = scanner->getNext();
        }
        
        // Load the include path
        // TODO: We need better path support
#ifdef DEV_LINK_MODE
        path = std::string(ORKA_HEADER_LOCATION) + "/" + path + ".oh";
#else
        path = "/usr/local/include/orka/" + path + ".oh";
#endif
        std::string preprocInclude = preprocessFile(path, print);
        
        std::ifstream reader(preprocInclude);
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
    
    return newPath;
}

