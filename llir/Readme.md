## LLIR

LLIR is a backend framework similar in design and purpose to LLVM. It is designed to be easily used by compiler frontends, and to be easily extendable to new architectures. Similar to LLVM, LLIR can be used in two ways: as a C++ framework, or through the midend IR.

### Why?

Why create this when we have LLVM?

The short answer: LLVM is huge.

After creating Tiny Lang, and thus having my own programming language for easy use in different areas, I found that I really needed a backend for certain cases (especially experimenting with exotic architectures). Yes, LLVM in theory can be used for these cases, but again LLVM is huge and I really didn't want to deal with that. So, I created this framework. It's very simple at the moment- as of the first version, it only supports x86-64, but it is ready to be extended and used in different areas.

### Hello World in IR

A simple Hello World program in the IR:

```
#module first

extern void puts(%0:*i8);

global i32 main() {
entry:
  call void puts($STR0("Hi!"));
  ret i32 2;
}
```

### A simple C++ example

This creates a simple function that returns 0. Feel free to change the constant to make it return other things:

```c++
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

#include <llir.hpp>
#include <irbuilder.hpp>
#include <amd64/amd64.hpp>
using namespace LLIR;

int main(int argc, char **argv) {
    Module *mod = new Module("test1");
    IRBuilder *builder = new IRBuilder(mod);
    
    //
    // func main:
    //     ret 0
    Type *i32Type = Type::createI32Type();
    Function *mainFunc = Function::Create("main", Linkage::Global, i32Type);
    mod->addFunction(mainFunc);
    builder->setCurrentFunction(mainFunc);
    builder->createBlock("entry");
    
    Operand *i32 = builder->createI32(0);
    builder->createRet(i32Type, i32);
    
    mod->print();
    mod->transform();
    
    // Generate a binary
    mkdir("./test_bin", 0700);
    
    LLIR::Amd64Writer *writer = new LLIR::Amd64Writer(mod);
    writer->compile();
    writer->writeToFile("/tmp/test1.s");
    system("gcc /tmp/test1.s -o ./test_bin/test1");
    
    return 0;
}
```
