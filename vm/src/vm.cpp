//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <fstream>
#include <cstdio>

#include "vm.hpp"

//
// Loads the file
//
void VM::load(std::string path, bool debug) {
    if (debug) {
        std::cout << "[SETVM] Loading the binary file..." << std::endl;
    }
    
    std::ifstream reader("./first.bin");
    if (!reader.is_open()) {
        std::cerr << "[SETVM] Error: Unable to open binary file." << std::endl;
        return;
    }
    
    while (!reader.eof()) {
        uint8_t c = reader.get();
        if (reader.eof()) break;
        code.push_back(c);
    }
    
    reader.close();
    
    // Debug if needed
    if (debug) {
        for (uint8_t op : code) {
            printf("%X ", op);
        }
        std::cout << std::endl;
    }
}

//
// The VM run loop
//
void VM::run(bool debug) {
    if (debug) {
        std::cout << "[SETVM] Running program..." << std::endl;
    }
    
    // Run the program
    while (pc < code.size()) {
        uint8_t opcode = code.at(pc);
        ++pc;
        
        switch (opcode) {
            // Allocates a set in memory
            case ALLOC_SET: {
                // Get the memory location
                uint8_t pos = code.at(pc);
                ++pc;
                
                ilist_memory[pos] = std::vector<uint64_t>();
            } break;
            
            // Adds an element to a set
            case SET_ADD_CONST: {
                // Get the data type
                uint8_t dtype = code.at(pc);
                ++pc;
                
                // Get the memory location
                uint8_t pos = code.at(pc);
                ++pc;
                
                // Get the constant
                uint8_t num = code.at(pc);
                ++pc;
                
                // Store the element
                ilist_memory[pos].push_back(num);
                
                // Debug
                if (debug) {
                    auto elements = ilist_memory[pos];
                    printf("DEBUG: %d = {", pos);
                    for (auto e : elements) {
                        printf("%ld ", e);
                    }
                    printf("}\n");
                }
            } break;
            
            // Prints a set
            case SET_PRINT: {
                // Get the data type
                uint8_t dtype = code.at(pc);
                ++pc;
                
                // Get the location
                uint8_t pos = code.at(pc);
                ++pc;
                
                auto elements = ilist_memory[pos];
                std::cout << "{";
                for (auto e : elements) {
                    std::cout << e << " ";
                }
                std::cout << "}" << std::endl;
            } break;
            
            // Creates a set intersection
            case SET_INTER: {
                // Get the data type
                uint8_t dtype = code.at(pc);
                ++pc;
                
                // Get the destination
                uint8_t rd = code.at(pc);
                ++pc;
                
                // Get the first source
                uint8_t rs1 = code.at(pc);
                ++pc;
                
                // Get the second source
                uint8_t rs2 = code.at(pc);
                ++pc;
                
                // Compute the intersection
                auto rd_elements = ilist_memory[rd];
                auto rs1_elements = ilist_memory[rs1];
                auto rs2_elements = ilist_memory[rs2];
                for (auto e1 : rs1_elements) {
                    for (auto e2 : rs2_elements) {
                        if (e1 == e2) {
                            rd_elements.push_back(e1);
                        }
                    }
                }
                ilist_memory[rd] = rd_elements;
            } break;
            
            // Creates a set union
            case SET_UNION: {
                // Get the data type
                uint8_t dtype = code.at(pc);
                ++pc;
                
                // Get the destination
                uint8_t rd = code.at(pc);
                ++pc;
                
                // Get the first source
                uint8_t rs1 = code.at(pc);
                ++pc;
                
                // Get the second source
                uint8_t rs2 = code.at(pc);
                ++pc;
                
                // Compute the union
                auto rd_elements = ilist_memory[rd];
                auto rs1_elements = ilist_memory[rs1];
                auto rs2_elements = ilist_memory[rs2];
                for (auto e1 : rs1_elements) {
                    bool found = false;
                    for (auto e2 : rd_elements) {
                        if (e1 == e2) {
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        rd_elements.push_back(e1);
                    }
                }
                for (auto e1 : rs2_elements) {
                    bool found = false;
                    for (auto e2 : rd_elements) {
                        if (e1 == e2) {
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        rd_elements.push_back(e1);
                    }
                }
                ilist_memory[rd] = rd_elements;
            } break;
            
            // Unknown opcode
            default: {
                std::cerr << "[SETVM] Error: Unknown opcode." << std::endl;
                printf("--> %x\n", opcode);
            }
        }
    }
}

