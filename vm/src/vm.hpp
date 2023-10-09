#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdio>

#define ALLOC_SET       0x30
#define SET_ADD_CONST   0x40
#define SET_PRINT       0x41
#define SET_UNION       0x50
#define SET_INTER       0x51

struct VM {
    // Set memory
    std::map<int, std::vector<uint64_t>> ilist_memory;
    
    // The code
    std::vector<uint8_t> code;
    int pc = 0;
    
    //
    // Functions
    //
    void load(std::string path, bool debug = false);
    void run(bool debug = false);
};


