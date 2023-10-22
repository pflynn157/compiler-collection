//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <JavaCompiler.hpp>

std::string GetClassName(std::string input) {
    int startPos = 0;
    int endPos = 0;

    // Determine start position
    for (int i = 0; i<input.length(); i++) {
        if (input[i] == '/') startPos = i + 1;
    }
    
    // Determine end position
    for (int i = input.length() - 1; i >= 0; i--) {
        if (input[i] == '.') {
            endPos = i;
            break;
        }
    }
    
    // Get the class name
    std::string name = "";
    
    for (int i = startPos; i<endPos; i++) {
        name += input[i];
    }
    
    return name;
}
