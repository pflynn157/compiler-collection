#include <LLIR/LLIRBuilder.hpp>
#include <LLIR/LLIR.hpp>

LLIRImm *LLIRBuilder::getInt32(int value) {
    LLIRType *i32 = new LLIRType(LLIRDataType::I32);
    LLIRImm *imm = new LLIRImm(value, i32);
    return imm;
}

LLIRInstruction *LLIRBuilder::CreateRet(LLIRValue *value) {
    std::string name = std::to_string(valCount);
    ++valCount;
    
    LLIRInstruction *ret = new LLIRInstruction(name, value->getType(), LLIR::Ret);
    ret->setArg1(value);
    insertBlock->addInstruction(ret);
    return ret;
}
