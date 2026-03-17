//
// Created by prone on 17.03.2026.
//

#include "offsetExtractor.h"
offsetExtractor* offsetExtractor::instance = nullptr;

offsetExtractor::offsetExtractor()
{
    jvmModule = GetModuleHandleA("jvm.dll");
    assert(jvmModule, "can't find jvm module");

    gHotSpotVMStructs = GetProcAddress(jvmModule, "gHotSpotVMStructs");
    assert(gHotSpotVMStructs, "can't find gHotSpotVMStructs proc handle");

    auto** hotSpotVMEntries = (VMStructEntry**)gHotSpotVMStructs;

    VMStructEntry* entry = *hotSpotVMEntries;
    while (entry->typeName != nullptr) {
        std::flush(std::cout);
        VMStructs.push_back(*entry);
        entry++;
    }
}

VMStructEntry* offsetExtractor::findVMStructEntry(const char* typeName, const char* fieldName)
{
    for (auto& entry : VMStructs) {
        std::cout << entry.typeName << " " << entry.fieldName  << std::endl;
        if (strcmp(entry.typeName, typeName) == 0 &&
            strcmp(entry.fieldName, fieldName) == 0 ){
            return &entry;
            }
    }
    return nullptr;
}

std::vector<VMStructEntry> offsetExtractor::getVMStructs()
{
    return VMStructs;
}

offsetExtractor* offsetExtractor::getInstance()
{
    if (!instance)
    {
        instance = new offsetExtractor();
    }
    return instance;
}


