//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <cstdio>
#include <arpa/inet.h>

#include <Java/JavaIR.hpp>

void JavaClassRef::write(FILE *file) {
    fputc(tag, file);
    fwrite(&nameIndex, sizeof(short), 1, file);
}

void JavaUTF8Entry::write(FILE *file) {
    fputc(tag, file);

    unsigned short length = htons(data.length());
    fwrite(&length, sizeof(short), 1, file);

    for (char c : data) fputc(c, file);
}

void JavaStringEntry::write(FILE *file) {
    fputc(tag, file);
    fwrite(&nameIndex, sizeof(short), 1, file);
}

void JavaFieldRefEntry::write(FILE *file) {
    fputc(tag, file);

    classIndex = htons(classIndex);
    ntIndex = htons(ntIndex);

    fwrite(&classIndex, sizeof(short), 1, file);
    fwrite(&ntIndex, sizeof(short), 1, file);
}

void JavaMethodRefEntry::write(FILE *file) {
    fputc(tag, file);

    classIndex = htons(classIndex);
    ntIndex = htons(ntIndex);

    fwrite(&classIndex, sizeof(short), 1, file);
    fwrite(&ntIndex, sizeof(short), 1, file);
}

void JavaNameTypeEntry::write(FILE *file) {
    fputc(tag, file);

    nameIndex = htons(nameIndex);
    descIndex = htons(descIndex);

    fwrite(&nameIndex, sizeof(short), 1, file);
    fwrite(&descIndex, sizeof(short), 1, file);
}

void JavaCodeBlock::write(FILE *file) {
    codeIdx = htons(codeIdx);

    unsigned int codeSize = 0;
    for (JavaCode c : code) codeSize += c.size();
    size = htonl(codeSize + size);
    codeSize = htonl(codeSize);

    fwrite(&codeIdx, sizeof(short), 1, file);
    fwrite(&size, sizeof(int), 1, file);
    fwrite(&stackSize, sizeof(short), 1, file);
    fwrite(&maxVars, sizeof(short), 1, file);

    fwrite(&codeSize, sizeof(int), 1, file);
    for (JavaCode c : code) c.write(file);

    fwrite(&exceptionSize, sizeof(short), 1, file);
    fwrite(&attrSize, sizeof(short), 1, file);
}
