//
// Created by prone on 02.08.2026.
//

#ifndef CUSTOM_JNA_JVMFLAG_H
#define CUSTOM_JNA_JVMFLAG_H

#include "../../../types.h"

struct JVMFlag
{
    void* _addr;
    const char* _name;
    int _flags;
    int _type;
};

#endif //CUSTOM_JNA_JVMFLAG_H
