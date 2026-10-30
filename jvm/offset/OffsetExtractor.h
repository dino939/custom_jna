//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_OFFSETEXTRACTOR_H
#define CUSTOM_JNA_OFFSETEXTRACTOR_H

#include "../struct/vmStructs.hpp"
#include "../struct/runtime/flags/jvmFlag.hpp"
#include <vector>
#include "../utils/assert/assert.hpp"
#include <windows.h>

class OffsetExtractor
{
    private:
    static OffsetExtractor* instance;
    std::vector<VMStructEntry> VMStructs;
    std::vector<VMTypeEntry> VMTypes;
    HMODULE jvmModule;
    FARPROC gHotSpotVMStructs;
    FARPROC gHotSpotVMTypes;
    JVMFlag* jvmFlags;
    size_t jvmFlagCount;

public:
    OffsetExtractor();
    VMStructEntry* findVMStructEntry(const char* typeName, const char* fieldName);
    VMTypeEntry* findVMTypeEntry(const char* typeName);
    std::vector<VMStructEntry> getVMStructs();
    std::vector<VMTypeEntry> getVMTypes();
    static OffsetExtractor* getInstance();
    JVMFlag* getJVMFlags();
    size_t getJVMFlagCount();
    JVMFlag* findJVMFlag(const char* name);
};


#endif //CUSTOM_JNA_OFFSETEXTRACTOR_H
