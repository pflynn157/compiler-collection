//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <fstream>
#include <iostream>
#include <cstdio>

std::string getInputPath(std::string input) {
    std::string name = "";
    for (int i = 0; i<input.length() - 3; i++) {
        char c = input.at(i);
        if (c == '/') name = "";
        else if (c == '.') continue;
        else name += c;
    }
    
    name += "_pre.tl";
    return name;
}

std::string preprocessFile(std::string input, bool print) {
    return input;
    /*std::string newPath = "/tmp/" + getInputPath(input);
    
    FILE *file = fopen(input.c_str(), "r");
    std::string input_str = "";
    while (!feof(file)) {
        input_str += fgetc(file);
    }
    Scanner *scanner = lex_init_string((char *)input_str.c_str());
    
    std::string content = "";
    
    // Read until the end of the file
    token tk;
    while (tk != t_eof) {
        tk = lex_get_next(scanner);
        
        if (token.type != Import) {
            content += scanner->getRawBuffer();
            continue;
        }
        
        // Indicate we have an import line
        content += "#pragma count\n";
        
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
        path = "/usr/local/include/tinylang/" + path + ".th";
        std::string preprocInclude = preprocessFile(path, false);
        
        std::ifstream reader(preprocInclude.c_str());
        if (!reader.is_open()) {
            std::cerr << "Fatal: Unable to open preprocessed include" << std::endl;
            return "";
        }
        
        std::string line = "";
        while (std::getline(reader, line)) {
            if (line == "" || line.length() == 0) continue;
            content += "#pragma nocount\n";
            content += line + "\n";
        }
        int last = content.length() - 1;
        if (content[last] == '\n') content[last] = ' ';
        
        reader.close();
        remove(preprocInclude.c_str());
        
        // Drop the buffer so we don't put the include line back in
        scanner->getRawBuffer();
    }
    
    if (print) std::cout << content << std::endl;
    
    std::ofstream writer(newPath, std::ios_base::out | std::ios_base::trunc);
    if (writer.is_open()) {
        writer << content;
        writer.close();
    } else {
        std::cerr << "Unable to open new file in preproc" << std::endl;
    }
    
    delete scanner;
    return newPath;*/
}

