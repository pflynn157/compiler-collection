#include <LLIR/LLIRCompiler.hpp>
#include <LLIR/LLIR.hpp>
#include <LLIR/PASM.hpp>
#include <LLIR/X86Writer.hpp>

LLIRCompiler::LLIRCompiler(AstTree *tree, std::string name) {
    this->tree = tree;
    this->name = name;
}

void LLIRCompiler::compile() {
    llir = new LLIRFile("output1");
    builder = new LLIRBuilder;
    compileLLIR();
    llir->dump();
    
    // PASM
    compilePASM();
    
    // PASM
    /*PASMFile *pasm = new PASMFile("output1");
    
    PASMFunc *mainFunc = new PASMFunc("main", 0);
    pasm->addCode(mainFunc);
    
    PASMInstruction *li = new PASMInstruction(PASM::LI);
    li->setOperand1(new PASMReg32(0));
    li->setOperand2(new PASMImm(23));
    pasm->addCode(li);
    
    PASMInstruction *ret = new PASMInstruction(PASM::RET);
    pasm->addCode(ret);
    */
    //pasm->debug();
    
    //x86
    X86Writer *writer = new X86Writer(pasm);
    writer->compile();
    writer->assemble();
    writer->link();
}

void LLIRCompiler::debug() {

}

//
// Converts the AST to an LLIR file
//
void LLIRCompiler::compileLLIR() {
    /*LLIRType *voidType = new LLIRType(LLIRDataType::Void);
    LLIRType *i32 = new LLIRType(LLIRDataType::I32);
    
    LLIRFunction *func = new LLIRFunction("main", voidType);
    llir->addFunction(func);
    
    LLIRBlock *block = new LLIRBlock("entry");
    func->setEntryBlock(block);
    
    LLIRImm *val = new LLIRImm(23, i32);
    LLIRInstruction *ret = new LLIRInstruction("", i32, LLIR::Ret);
    ret->setArg1(val);
    block->addInstruction(ret);*/
    
    // Build all other functions
    for (auto global : tree->getGlobalStatements()) {
        switch (global->getType()) {
            case AstType::Func: {
                compileFunction(global);
            } break;
            
            case AstType::ExternFunc: {
                //TODO
            } break;
        
            default: {}
        }
    }
}

//
// Converts the AST function to the an function
//
void LLIRCompiler::compileFunction(AstGlobalStatement *global) {
    AstFunction *astFunc = static_cast<AstFunction *>(global);
    
    LLIRType *funcType = translateType(astFunc->getDataType(), astFunc->getPtrType());
    LLIRFunction *func = new LLIRFunction(astFunc->getName(), funcType);
    llir->addFunction(func);
    
    LLIRBlock *block = new LLIRBlock("entry");
    func->setEntryBlock(block);
    builder->setInsertBlock(block);
    
    for (auto stmt : astFunc->getBlock()->getBlock()) {
        compileStatement(stmt);
    }
}

void LLIRCompiler::compileStatement(AstStatement *stmt) {
    switch (stmt->getType()) {
        // A return statement
        case AstType::Return: {
            if (stmt->getExpressionCount() == 1) {
                LLIRValue *val = compileValue(stmt->getExpressions().at(0));
                builder->CreateRet(val);
            } else {
            
            }
        } break;
        
        default: {}
    }
}

LLIRValue *LLIRCompiler::compileValue(AstExpression *expr) {
    switch (expr->getType()) {
        case AstType::IntL: {
            AstInt *ival = static_cast<AstInt *>(expr);
            return builder->getInt32(ival->getValue());
        } break;
        
        default: {}
    }
    
    return nullptr;
}

LLIRType *LLIRCompiler::translateType(DataType dataType, DataType subType) {
    LLIRType *type;
    
    switch (dataType) {
        case DataType::Int32:
        case DataType::UInt32: type = new LLIRType(LLIRDataType::I32); break;
        
        case DataType::Void: type = new LLIRType(LLIRDataType::Void); break;
    
        default: {}
    }
    
    return type;
}

//
// Converts the LLIR to PASM
//
void LLIRCompiler::compilePASM() {
    pasm = new PASMFile("output1");
    
    for (LLIRFunction *func : llir->getFunctions()) {
        PASMFunc *f = new PASMFunc(func->getName(), 0);
        pasm->addCode(f);
        
        LLIRBlock *block = func->getEntryBlock();
        while (block != nullptr) {
            PASMLabel *lbl = new PASMLabel(block->getName());
            pasm->addCode(lbl);
            
            // Build the code
            for (LLIRInstruction *instr : block->getCode()) {
                switch (instr->getOpType()) {
                    case LLIR::Ret: {
                        LLIRValue *retVal = instr->getArg1();
                        
                        switch (retVal->getOpType()) {
                            case LLIR::Imm: {
                                LLIRImm *imm = static_cast<LLIRImm *>(retVal);
                            
                                PASMInstruction *li = new PASMInstruction(PASM::LI);
                                li->setOperand1(new PASMReg32(0));
                                li->setOperand2(new PASMImm(imm->getValue()));
                                pasm->addCode(li);
                                
                                PASMInstruction *ret = new PASMInstruction(PASM::RET);
                                pasm->addCode(ret);  
                            } break;
                            
                            default: {}
                        }
                    } break;
                    
                    default: {}
                }
            }
            
            // Get the next block
            block = block->getNext();
        }
    }
}
