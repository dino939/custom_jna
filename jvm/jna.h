//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_JNA_H
#define CUSTOM_JNA_JNA_H
#include "types.h"

typedef InstanceKlass* (*FindCLassPtr)(const char* name);

struct JNINativeAccess_ {
    FindCLassPtr findClass;
};

typedef JNINativeAccess_* environment;

#endif //CUSTOM_JNA_JNA_H