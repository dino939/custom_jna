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

    gHotSpotVMTypes = GetProcAddress(jvmModule, "gHotSpotVMTypes");
    assert(gHotSpotVMTypes, "can't find gHotSpotVMTypes proc handle");

    auto** hotSpotVMEntries = (VMStructEntry**)gHotSpotVMStructs;

    VMStructEntry* entry = *hotSpotVMEntries;
    while (entry->typeName != nullptr) {
        VMStructs.push_back(*entry);
        entry++;
    }

    auto** hotSpotVMTypeEntries = (VMTypeEntry**)gHotSpotVMTypes;

    VMTypeEntry* vmTypeLocal = *hotSpotVMTypeEntries;
    while (vmTypeLocal->type_name != nullptr) {
        VMTypes.push_back(*vmTypeLocal);
        vmTypeLocal++;
    }

    jvmFlags = *static_cast<JVMFlag**>(findVMStructEntry("JVMFlag", "flags")->address);
    jvmFlagCount = *static_cast<size_t*>(findVMStructEntry("JVMFlag", "numFlags")->address);
    assert(jvmFlags, "can't resolve JVMFlag::flags");
}

VMStructEntry* OffsetExtractor::findVMStructEntry(const char* typeName, const char* fieldName)
{
    for (auto& entry : VMStructs) {
        if (strcmp(entry.typeName, typeName) == 0 &&
            strcmp(entry.fieldName, fieldName) == 0 ){
            return &entry;
            }
    }
    static auto dummy = VMStructEntry(nullptr,nullptr,nullptr,0,-1,nullptr);
    return &dummy;
}
VMTypeEntry* OffsetExtractor::findVMTypeEntry(const char* typeName)
{
    for (auto& entry : VMTypes) {
        if (strcmp(entry.type_name, typeName) == 0){
            return &entry;
            }
    }
    return nullptr;
}


std::vector<VMStructEntry> OffsetExtractor::getVMStructs()
{
    return VMStructs;
}

std::vector<VMTypeEntry> OffsetExtractor::getVMTypes()
{
    return VMTypes;
}

OffsetExtractor* OffsetExtractor::getInstance()
{
    if (!instance)
    {
        instance = new OffsetExtractor();
    }
    return instance;
}

JVMFlag* OffsetExtractor::getJVMFlags()
{
    return jvmFlags;
}

size_t OffsetExtractor::getJVMFlagCount()
{
    return jvmFlagCount;
}

JVMFlag* OffsetExtractor::findJVMFlag(const char* name)
{
    for (size_t i = 0; i < jvmFlagCount; i++) {
        JVMFlag* flag = &jvmFlags[i];
        if (flag->_name != nullptr && strcmp(flag->_name, name) == 0) {
            return flag;
        }
    }
    return nullptr;
}


