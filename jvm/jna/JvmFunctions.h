//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_JVMFUNCTIONS_H
#define CUSTOM_JNA_JVMFUNCTIONS_H
#include "../struct/klass/InstanceKlass.h"
#include "../types.h"
#include "../utils/assert/assert.hpp"
#include "../struct/classloader/ClassLoaderData.h"
class JvmFunctions
{
public:
   static InstanceKlass* findKlass17JVM(const char* name);
};


#endif //CUSTOM_JNA_JVMFUNCTIONS_H