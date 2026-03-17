//
// Created by prone on 17.03.2026.
//

#include "offsetExtractor.h"
OffsetExtractor* OffsetExtractor::instance = nullptr;

OffsetExtractor::OffsetExtractor()
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

VMStructEntry* OffsetExtractor::findVMStructEntry(const char* typeName, const char* fieldName)
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

std::vector<VMStructEntry> OffsetExtractor::getVMStructs()
{
    return VMStructs;
}

OffsetExtractor* OffsetExtractor::getInstance()
{
    if (!instance)
    {
        instance = new OffsetExtractor();
    }
    return instance;
}


