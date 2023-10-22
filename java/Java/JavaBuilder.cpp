//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <memory>

#include <Java/JavaIR.hpp>
#include <Java/JavaBuilder.hpp>

// Sets initial things up
JavaClassBuilder::JavaClassBuilder(std::string className) {
    java = std::make_shared<JavaClassFile>();
    this->className = className;

    // Sets the class name
    int pos = AddUTF8(className);
    auto classRef = std::make_shared<JavaClassRef>(pos);
    pos = java->AddConst(classRef);
    classMap[className] = pos;
    superPos = pos;

    java->this_idx = htons(pos);

    // Set the super class
    pos = AddUTF8("java/lang/Object");
    auto superRef = std::make_shared<JavaClassRef>(pos);
    pos = java->AddConst(superRef);
    classMap["java/lang/Object"] = pos;

    java->super_idx = htons(pos);

    codeIdx = AddUTF8("Code");

    // Import the object constructor
    ImportMethod("java/lang/Object", "<init>", "()V");
}

// Adds a utf8 string to the constant pool
int JavaClassBuilder::AddUTF8(std::string value) {
    auto entry = std::make_shared<JavaUTF8Entry>(value);
    java->const_pool.push_back(entry);

    int pos = java->const_pool.size();
    UTF8Index[value] = pos;

    return pos;
}

// Imports a class
int JavaClassBuilder::ImportClass(std::string baseClass) {
   int classPos = 0;

    if (classMap.find(baseClass) == classMap.end()) {
        classPos = AddUTF8(baseClass);

        auto classRef = std::make_shared<JavaClassRef>(classPos);
        classPos = java->AddConst(classRef);

        classMap[baseClass] = classPos;
    } else {
        classPos = classMap[baseClass];
    }

    return classPos;
}

// Imports a method
void JavaClassBuilder::ImportMethod(std::string baseClass, std::string name, std::string signature) {
    int classPos = ImportClass(baseClass);

    // Create the name and type <name><type>
    int namePos = AddUTF8(name);
    int sigPos = AddUTF8(signature);

    auto nt = std::make_shared<JavaNameTypeEntry>(namePos, sigPos);
    int ntPos = java->AddConst(nt);

    // Create the method ref <class><name_type>
    auto method = std::make_shared<JavaMethodRefEntry>(classPos, ntPos);
    int methodPos = java->AddConst(method);

    //methodMap[name] = methodPos;
    Method m(name, methodPos, baseClass, signature);
    methodMap.push_back(m);
}

// Imports a field from another class
void JavaClassBuilder::ImportField(std::string baseClass, std::string typeClass, std::string name) {
    int baseClassPos = ImportClass(baseClass);
    int typeClassPos = ImportClass(typeClass);

    // Create the signature: "L<typeClass>;"
    std::string sig = "L" + typeClass + ";";
    int sigPos = AddUTF8(sig);

    int namePos = AddUTF8(name);

    // NameAndType <name><sig>
    auto nt = std::make_shared<JavaNameTypeEntry>(namePos, sigPos);
    int ntPos = java->AddConst(nt);

    // FieldRef <baseClass><name_type>
    auto ref = std::make_shared<JavaFieldRefEntry>(baseClassPos, ntPos);
    int refPos = java->AddConst(ref);

    fieldMap[name] = refPos;
}

// Finds a method in the method table
int JavaClassBuilder::FindMethod(std::string name, std::string baseClass, std::string signature) {
    for (auto m : methodMap) {
        if (m.name == name) {
            bool found1 = true;
            bool found2 = true;
            
            if (baseClass != "") {
                if (m.baseClass != baseClass && baseClass != "this") found1 = false;
            }
            
            if (signature != "") {
                if (m.signature != signature) found2 = false;
            }
            
            if (found1 && found2) return m.pos;
        }
    }

    return 0;
}

// Writes out the class file
void JavaClassBuilder::Write(FILE *file) {
    java->write(file);
}
