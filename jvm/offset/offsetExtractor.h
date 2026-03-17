//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_OFFSETEXTRACTOR_H
#define CUSTOM_JNA_OFFSETEXTRACTOR_H

#include "../struct/vmStructs.hpp"
#include <vector>
#include "../utils/assert/assert.hpp"
#include <windows.h>

class offsetExtractor
{
    private:
    static offsetExtractor* instance;
    std::vector<VMStructEntry> VMStructs;
    HMODULE jvmModule;
    FARPROC gHotSpotVMStructs;

public:
    offsetExtractor();
    VMStructEntry* findVMStructEntry(const char* typeName, const char* fieldName);
    std::vector<VMStructEntry> getVMStructs();
    static offsetExtractor* getInstance();
};


#endif //CUSTOM_JNA_OFFSETEXTRACTOR_H
