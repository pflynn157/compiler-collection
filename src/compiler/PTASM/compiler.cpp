#include <memory>

#include "compiler.hpp"
#include "ptasm.hpp"

Compiler::Compiler(std::string name) {
    file = new AsmFile(name);
}

void Compiler::compile(std::shared_ptr<AstTree> tree) {
    // Start by compiling global statements
    for (auto const &global : tree->getGlobalStatements()) {
        if (global->getType() == V_AstType::ExternFunc) {
        
        } else if (global->getType() == V_AstType::Func) {
            compileFunction(global);
        }
    }
}

void Compiler::compileFunction(std::shared_ptr<AstGlobalStatement> global) {
    std::shared_ptr<AstFunction> func = std::static_pointer_cast<AstFunction>(global);
    AsmFunction *asmFunc = new AsmFunction(func->getName());
    file->addFunction(asmFunc);
    
    // Now, we have to construct the block
    compileBlock(asmFunc, func->getBlock());
}

void Compiler::compileBlock(AsmFunction *func, std::shared_ptr<AstBlock> block) {
    std::string blockName = "block" + std::to_string(label_counter);
    ++label_counter;
    AsmBlock *asmBlock = new AsmBlock(blockName);
    func->addBlock(asmBlock);
    
    // Now, assemble each statement within the block
    for (auto const &stmt : block->getBlock()) {
        compileStatement(func, asmBlock, stmt);
    }
}

void Compiler::compileStatement(AsmFunction *func, AsmBlock *block, std::shared_ptr<AstStatement> stmt) {
    switch (stmt->getType()) {
        case V_AstType::VarDec: {
            std::shared_ptr<AstVarDec> vd = std::static_pointer_cast<AstVarDec>(stmt);
        
            AsmInstruction *instr = new AsmInstruction(V_AsmType::Alloca);
            instr->setDestOperand(new AsmId(vd->getName()));
            instr->setDataType(new AsmType(V_AsmType::I32));        // TODO: Change
            block->addInstruction(instr);
        } break;
    
        case V_AstType::Return: {
            AsmInstruction *instr = new AsmInstruction(V_AsmType::Ret);
            instr->setDestOperand(new AsmOperand(V_AsmType::RetReg));
            block->addInstruction(instr);
            
            if (stmt->hasExpression()) {
                AsmOperand *src1 = compileExpression(block, stmt->getExpression());
                instr->setSrc1Operand(src1);
            }
            
            // TODO: Determine
            instr->setDataType(new AsmType(V_AsmType::I32));
        } break;
    
        default: {}
    }
}

AsmOperand *Compiler::compileExpression(AsmBlock *block, std::shared_ptr<AstExpression> expr) {
    switch (expr->getType()) {
        case V_AstType::I32L: {
            std::shared_ptr<AstI32> i = std::static_pointer_cast<AstI32>(expr);
            return new AsmInt(i->getValue());
        }
        
        default: {}
    }
    
    return nullptr;
}

