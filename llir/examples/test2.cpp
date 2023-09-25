#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

#include <llir.hpp>
#include <irbuilder.hpp>
#include <amd64/amd64.hpp>
using namespace LLIR;

int main(int argc, char **argv) {
    Module *mod = new Module("test2");
    IRBuilder *builder = new IRBuilder(mod);
    
    //
    // func main:
    //     %0 = alloca i32
    //     %1 = alloca i32
    //     %2 = alloca i32
    //     store 20, %0
    //     store 30, %1
    //     %4 = load %0
    //     %5 = load %1
    //     %6 = add %4, %5
    //     store %6, %2
    //     %7 = load %2
    //     %8 = sub %7, 50
    //     ret %8
    Type *i32Type = Type::createI32Type();
    Function *mainFunc = Function::Create("main", Linkage::Global, i32Type);
    mod->addFunction(mainFunc);
    builder->setCurrentFunction(mainFunc);
    builder->createBlock("entry");
    
    //     %0 = alloca i32
    //     %1 = alloca i32
    //     %2 = alloca i32
    Reg *r0 = builder->createAlloca(i32Type);
    Reg *r1 = builder->createAlloca(i32Type);
    Reg *r2 = builder->createAlloca(i32Type);
    
    //     store 20, %0
    //     store 30, %1
    Operand *imm1 = builder->createI32(20);
    Operand *imm2 = builder->createI32(30);
    builder->createStore(i32Type, imm1, r0);
    builder->createStore(i32Type, imm2, r1);
    
    //     %4 = load %0
    //     %5 = load %1
    //     %6 = add %4, %5
    //     store %6, %2
    Operand *r4 = builder->createLoad(i32Type, r0);
    Operand *r5 = builder->createLoad(i32Type, r1);
    Operand *r6 = builder->createAdd(i32Type, r4, r5);
    builder->createStore(i32Type, r6, r2);
    
    //     %7 = load %2
    //     %8 = sub %7, 50
    //     ret %8
    Operand *r7 = builder->createLoad(i32Type, r2);
    Operand *r8 = builder->createSub(i32Type, r7, builder->createI32(50));
    builder->createRet(i32Type, r8);
    
    mod->print();
    mod->transform();
    
    // Generate a binary
    mkdir("./test_bin", 0700);
    
    LLIR::Amd64Writer *writer = new LLIR::Amd64Writer(mod);
    writer->compile();
    writer->writeToFile("/tmp/test2.s");
    system("gcc /tmp/test2.s -o ./test_bin/test2");
    
    delete mod;
    delete builder;
    delete writer;
    
    return 0;
}
