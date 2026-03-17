#include <windows.h>
#include <iostream>
#include <fstream>
#include "jvm/offset/offsetExtractor.h"
#include "jvm/struct/klass/InstanceKlass.h"
#include "jvm/jna/JNABootStrap.h"

void entryPoint()
{
environment env = JNABootStrap::getInstance()->_env();
    for (VMStructEntry entry : offsetExtractor::getInstance()->getVMStructs())
    {
        std::cout << entry.typeName << " " << entry.fieldName << " " << entry.typeString << std::endl;
    }
}

DWORD WINAPI ConsoleThread(LPVOID)
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    std::cout << "Hello World!\n";
    entryPoint();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, ConsoleThread, nullptr, 0, nullptr);
        break;
    }
    return TRUE;
}
