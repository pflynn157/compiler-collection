#pragma once

#include <LLIR/LLIR.hpp>

class LLIRBuilder {
public:
    explicit LLIRBuilder() {}
    explicit LLIRBuilder(LLIRBlock *insertBlock) {
        this->insertBlock = insertBlock;
    }
    
    void setInsertBlock(LLIRBlock *insertBlock) {
        this->insertBlock = insertBlock;
    }
    
    // Constants
    LLIRImm *getInt32(int value);
    
    // Instructions
    LLIRInstruction *CreateRet(LLIRValue *value);
private:
    LLIRBlock *insertBlock;
    int valCount = 0;
};
