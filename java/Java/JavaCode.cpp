//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <Java/JavaBuilder.hpp>
#include <Java/JavaIR.hpp>

// Creates a static function
JavaFunction *JavaClassBuilder::CreateMethod(std::string name, std::string signature, int flags) {
    int nameIdx = AddUTF8(name);
    int sigIdx = AddUTF8(signature);

    JavaFunction *func = new JavaFunction(flags, nameIdx, sigIdx, codeIdx);
    java->methods.push_back(func);

    // Create a name and type entry
    JavaNameTypeEntry *nt = new JavaNameTypeEntry(nameIdx, sigIdx);
    int ntPos = java->AddConst(nt);

    // Create a method-ref entry
    JavaMethodRefEntry *method = new JavaMethodRefEntry(superPos, ntPos);
    int methodPos = java->AddConst(method);

    //methodMap[name] = methodPos;
    Method m(name, methodPos, className, signature);
    methodMap.push_back(m);

    return func;
}

// Creates an ALOAD instruction
void JavaClassBuilder::CreateALoad(JavaFunction *func, int pos) {
    switch (pos) {
        case 0: func->addCode(JavaCode(0x2A)); break;
        case 1: func->addCode(JavaCode(0x2B)); break;
        case 2: func->addCode(JavaCode(0x2C)); break;
        case 3: func->addCode(JavaCode(0x2D)); break;

        default: {
            func->addCode(JavaCode(0x19, (unsigned char)pos));
        }
    }
}

// Creates an ASTORE instruction
void JavaClassBuilder::CreateAStore(JavaFunction *func, int pos) {
    switch (pos) {
        case 0: func->addCode(JavaCode(0x4B)); break;
        case 1: func->addCode(JavaCode(0x4C)); break;
        case 2: func->addCode(JavaCode(0x4D)); break;
        case 3: func->addCode(JavaCode(0x4E)); break;

        default: {
            func->addCode(JavaCode(0x3A, (unsigned char)pos));
        }
    }
}

// Creates a NEW instruction
void JavaClassBuilder::CreateNew(JavaFunction *func, std::string name) {
    int pos = classMap[name];

    JavaCode code(0xBB, (unsigned short)pos);
    func->addCode(code);
}

// Creates a DUP instruction
void JavaClassBuilder::CreateDup(JavaFunction *func) {
    func->addCode(JavaCode(0x59));
}

// Creates a getstatic instruction
void JavaClassBuilder::CreateGetStatic(JavaFunction *func, std::string name) {
    int fieldPos = fieldMap[name];

    JavaCode code(0xB2, (unsigned short)fieldPos);
    func->addCode(code);
}

// Creates a LDC instruction (loads a string specifically)
void JavaClassBuilder::CreateString(JavaFunction *func, std::string value) {
    int constPos = 0;

    if (constMap.find(value) == constMap.end()) {
        constPos = AddUTF8(value);
        JavaStringEntry *entry = new JavaStringEntry(constPos);
        constPos = java->AddConst(entry);

        constMap[value] = constPos;
    } else {
        constPos = constMap[value];
    }

    JavaCode code(0x12, (unsigned char)constPos);
    func->addCode(code);
}

// Creates an InvokeSpecial instruction
void JavaClassBuilder::CreateInvokeSpecial(JavaFunction *func, std::string name, std::string baseClass, std::string signature) {
    int methodPos = FindMethod(name, baseClass, signature);

    JavaCode code(0xB7, (unsigned short)methodPos);
    func->addCode(code);
}

// Creates an InvokeVirtual instruction
void JavaClassBuilder::CreateInvokeVirtual(JavaFunction *func, std::string name, std::string baseClass, std::string signature) {
    int methodPos = FindMethod(name, baseClass, signature);

    JavaCode code(0xB6, (unsigned short)methodPos);
    func->addCode(code);
}

// Creates an InvokeStatic instruction
void JavaClassBuilder::CreateInvokeStatic(JavaFunction *func, std::string name, std::string baseClass, std::string signature) {
    int methodPos = FindMethod(name, baseClass, signature);

    JavaCode code(0xB8, (unsigned short)methodPos);
    func->addCode(code);
}

// Creates a return-void call
void JavaClassBuilder::CreateRetVoid(JavaFunction *func) {
    func->addCode(JavaCode(0xB1));
}

// Creates a bipush call
void JavaClassBuilder::CreateBIPush(JavaFunction *func, int value) {
    JavaCode code(0x10, (unsigned char)value);
    func->addCode(code);
}

// Creates an i_load call
void JavaClassBuilder::CreateILoad(JavaFunction *func, int value) {
    switch (value) {
        case 0: func->addCode(JavaCode(0x1A)); break;
        case 1: func->addCode(JavaCode(0x1B)); break;
        case 2: func->addCode(JavaCode(0x1C)); break;
        case 3: func->addCode(JavaCode(0x1D)); break;

        default: {
            func->addCode(JavaCode(0x15, (unsigned char)value));
        }
    }
}

// Creates an i_store call
void JavaClassBuilder::CreateIStore(JavaFunction *func, int value) {
    switch (value) {
        case 0: func->addCode(JavaCode(0x3B)); break;
        case 1: func->addCode(JavaCode(0x3C)); break;
        case 2: func->addCode(JavaCode(0x3D)); break;
        case 3: func->addCode(JavaCode(0x3E)); break;

        default: {
            func->addCode(JavaCode(0x36, (unsigned char)value));
        }
    }
}

// Creates an i_add instruction
void JavaClassBuilder::CreateIAdd(JavaFunction *func) {
    func->addCode(JavaCode(0x60));
}

// Creates an i_sub instruction
void JavaClassBuilder::CreateISub(JavaFunction *func) {
    func->addCode(JavaCode(0x64));
}

// Creates an i_mul instruction
void JavaClassBuilder::CreateIMul(JavaFunction *func) {
    func->addCode(JavaCode(0x68));
}

// Creates an i_div instruction
void JavaClassBuilder::CreateIDiv(JavaFunction *func) {
    func->addCode(JavaCode(0x6C));
}

// Creates an i_rem instruction
void JavaClassBuilder::CreateIRem(JavaFunction *func) {
    func->addCode(JavaCode(0x70));
}

// Creates an i_and instruction
void JavaClassBuilder::CreateIAnd(JavaFunction *func) {
    func->addCode(JavaCode(0x7E));
}

// Creates an i_or instruction
void JavaClassBuilder::CreateIOr(JavaFunction *func) {
    func->addCode(JavaCode(0x80));
}

// Creates an i_xor instruction
void JavaClassBuilder::CreateIXor(JavaFunction *func) {
    func->addCode(JavaCode(0x82));
}

// Creates an i_shl instruction
void JavaClassBuilder::CreateIShl(JavaFunction *func) {
    func->addCode(JavaCode(0x78));
}

// Creates an i_shr instruction
void JavaClassBuilder::CreateIShr(JavaFunction *func) {
    func->addCode(JavaCode(0x7A));
}
