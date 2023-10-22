//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <arpa/inet.h>
#include <map>
#include <vector>
#include <memory>

#include <Java/JavaIR.hpp>

struct Method {
    std::string name;
    std::string baseClass;
    std::string signature;
    int pos;

    Method(std::string name, int pos, std::string baseClass = "", std::string signature = "") {
        this->name = name;
        this->pos = pos;
        this->baseClass = baseClass;
        this->signature = signature;
    }
};

class JavaClassBuilder {
public:
    explicit JavaClassBuilder(std::string className);
    int AddUTF8(std::string value);
    int ImportClass(std::string baseClass);
    void ImportMethod(std::string baseClass, std::string name, std::string signature);
    void ImportField(std::string baseClass, std::string typeClass, std::string name);
    int FindMethod(std::string name, std::string baseClass, std::string signature);

    std::shared_ptr<JavaFunction> CreateMethod(std::string name, std::string signature, int = F_PUBLIC);
    void CreateALoad(std::shared_ptr<JavaFunction> func, int pos);
    void CreateAStore(std::shared_ptr<JavaFunction> func, int pos);
    void CreateNew(std::shared_ptr<JavaFunction> func, std::string name);
    void CreateDup(std::shared_ptr<JavaFunction> func);
    void CreateGetStatic(std::shared_ptr<JavaFunction> func, std::string name);
    void CreateString(std::shared_ptr<JavaFunction> func, std::string value);
    void CreateInvokeSpecial(std::shared_ptr<JavaFunction> func, std::string name, std::string baseClass = "", std::string signature = "");
    void CreateInvokeVirtual(std::shared_ptr<JavaFunction> func, std::string name, std::string baseClass = "", std::string signature = "");
    void CreateInvokeStatic(std::shared_ptr<JavaFunction> func, std::string name, std::string baseClass = "", std::string signature = "");
    void CreateRetVoid(std::shared_ptr<JavaFunction> func);
    
    // Integer instructions
    void CreateBIPush(std::shared_ptr<JavaFunction> func, int value);
    void CreateILoad(std::shared_ptr<JavaFunction> func, int value);
    void CreateIStore(std::shared_ptr<JavaFunction> func, int value);
    void CreateIAdd(std::shared_ptr<JavaFunction> func);
    void CreateISub(std::shared_ptr<JavaFunction> func);
    void CreateIMul(std::shared_ptr<JavaFunction> func);
    void CreateIDiv(std::shared_ptr<JavaFunction> func);
    void CreateIRem(std::shared_ptr<JavaFunction> func);
    void CreateIAnd(std::shared_ptr<JavaFunction> func);
    void CreateIOr(std::shared_ptr<JavaFunction> func);
    void CreateIXor(std::shared_ptr<JavaFunction> func);
    void CreateIShl(std::shared_ptr<JavaFunction> func);
    void CreateIShr(std::shared_ptr<JavaFunction> func);

    void Write(FILE *file);
private:
    std::shared_ptr<JavaClassFile> java;
    std::string className;
    int codeIdx = 0;
    int superPos = 0;

    std::map<std::string, int> UTF8Index;
    std::map<std::string, int> classMap;
    std::map<std::string, int> fieldMap;
    //std::map<std::string, int> methodMap;
    std::vector<Method> methodMap;
    std::map<std::string, int> constMap;
};
