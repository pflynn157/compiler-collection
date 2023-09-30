//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <arpa/inet.h>

enum JavaConstTag {
    UTF8 = 0x01,
    CLASS = 0x07,
    STRING = 8,
    FIELD_REF = 9,
    METHOD_REF = 10,
    NAME_AND_TYPE = 12
};

struct JavaConstEntry {
    unsigned char tag = 0;

    virtual void write(FILE *file) {}
};

// Represents a reference to a class
struct JavaClassRef : public JavaConstEntry {
    JavaClassRef(int stringPos) {
        this->tag = CLASS;
        this->nameIndex = htons(stringPos);
    }

    void write(FILE *file);
private:
    unsigned short nameIndex = 0;
};

// Represents a string entry
struct JavaStringEntry : public JavaConstEntry {
public:
    JavaStringEntry(int stringPos) {
        this->tag = STRING;
        this->nameIndex = htons(stringPos);
    }

    void write(FILE *file);
private:
    unsigned short nameIndex = 0;
};

// Represents a UTF-8 constant string
struct JavaUTF8Entry : public JavaConstEntry {
    JavaUTF8Entry(std::string data) {
        this->tag = UTF8;
        this->data = data;
    }

    void write(FILE *file);

private:
    std::string data;
};

// Represents a field-ref entry
struct JavaFieldRefEntry : public JavaConstEntry {
    JavaFieldRefEntry(unsigned short classIndex, unsigned short ntIndex) {
        this->tag = FIELD_REF;
        this->classIndex = classIndex;
        this->ntIndex = ntIndex;
    }

    void write(FILE *file);
private:
    unsigned short classIndex, ntIndex;
};

// Represents a method-ref entry
struct JavaMethodRefEntry : public JavaConstEntry {
    JavaMethodRefEntry(unsigned short classIndex, unsigned short ntIndex) {
        this->tag = METHOD_REF;
        this->classIndex = classIndex;
        this->ntIndex = ntIndex;
    }

    void write(FILE *file);
private:
    unsigned short classIndex, ntIndex;
};

// Represents a name-and-type entry
struct JavaNameTypeEntry : public JavaConstEntry {
    JavaNameTypeEntry(unsigned short nameIndex, unsigned short descIndex) {
        this->tag = NAME_AND_TYPE;
        this->nameIndex = nameIndex;
        this->descIndex = descIndex;
    }

    void write(FILE *file);
private:
    unsigned short nameIndex, descIndex;
};

struct JavaCode {
    unsigned char opcode;
    unsigned short arg1;            // argPos = 1
    unsigned char arg1_byte;        // argPos = 2

    int argPos = 0;

    JavaCode(unsigned char opcode) {
        this->opcode = opcode;
    }

    JavaCode(unsigned char opcode, unsigned short arg1) {
        this->opcode = opcode;
        this->arg1 = htons(arg1);
        argPos = 1;
    }

    JavaCode(unsigned char opcode, unsigned char arg1) {
        this->opcode = opcode;
        this->arg1_byte = arg1;
        argPos = 2;
    }

    int size() {
        if (argPos == 1) return 3;
        else if (argPos == 2) return 2;
        return 1;
    }

    void write(FILE *file) {
        fputc(opcode, file);
        if (argPos == 1) fwrite(&arg1, sizeof(short), 1, file);
        else if (argPos == 2) fputc(arg1_byte, file);
    }
};

struct JavaCodeBlock {
    unsigned short codeIdx = 0;
    unsigned int size = 12;   // Code size is added when written
    unsigned short stackSize = htons(5);
    unsigned short maxVars = htons(20);

    std::vector<JavaCode> code;

    unsigned short exceptionSize = 0;
    unsigned short attrSize = 0;

    void addCode(JavaCode c) { code.push_back(c); }

    void write(FILE *file);
};

enum JavaFuncFlags {
    F_PUBLIC = 0x0001,
    F_PRIVATE = 0x0002,
    F_PROTECTED = 0x0004,
    F_STATIC = 0x0008,
    F_FINAL = 0x0010
};

struct JavaFunction {
    JavaFunction(short flags, short nameIdx, short typeIdx, short codeIdx) {
        this->flags = htons(flags);
        this->nameIdx = htons(nameIdx);
        this->typeIdx = htons(typeIdx);
        codeBlock.codeIdx = codeIdx;
    }

    void addCode(JavaCode c) { codeBlock.addCode(c); }

    void write(FILE *file) {
        fwrite(&flags, sizeof(short), 1, file);
        fwrite(&nameIdx, sizeof(short), 1, file);
        fwrite(&typeIdx, sizeof(short), 1, file);
        fwrite(&attrCount, sizeof(short), 1, file);
        codeBlock.write(file);
    }

private:
    unsigned short flags = htons(0x0009);
    unsigned short nameIdx = 0;
    unsigned short typeIdx = 0;
    unsigned short attrCount = htons(1);

    JavaCodeBlock codeBlock;
};

struct JavaClassFile {
    unsigned int magic = htonl(0XCAFEBABE);
    unsigned short minor_version = 0;
    unsigned short major_version = htons(0x0034);

    std::vector<JavaConstEntry *> const_pool;

    unsigned short modifiers = htons(0x0021);
    unsigned short this_idx = 0;
    unsigned short super_idx = 0;

    unsigned short interface_count = 0;
    unsigned short field_count = 0;
    std::vector<JavaFunction *> methods;
    unsigned short attr_count = 0;

    int AddConst(JavaConstEntry *entry) {
        const_pool.push_back(entry);
        return const_pool.size();
    }

    void write(FILE *file) {
        fwrite(&magic, sizeof(int), 1, file);
        fwrite(&minor_version, sizeof(short), 1, file);
        fwrite(&major_version, sizeof(short), 1, file);

        unsigned short const_size = htons(const_pool.size() + 1);
        fwrite(&const_size, sizeof(short), 1, file);
        for (auto entry : const_pool) {
            entry->write(file);
        }

        fwrite(&modifiers, sizeof(short), 1, file);
        fwrite(&this_idx, sizeof(short), 1, file);
        fwrite(&super_idx, sizeof(short), 1, file);

        fwrite(&interface_count, sizeof(short), 1, file);
        fwrite(&field_count, sizeof(short), 1, file);

        unsigned short func_count = htons(methods.size());
        fwrite(&func_count, sizeof(short), 1, file);
        for (JavaFunction *func : methods) func->write(file);

        fwrite(&attr_count, sizeof(short), 1, file);
    }
};
