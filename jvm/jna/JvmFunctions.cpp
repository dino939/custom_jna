//
// Created by prone on 17.03.2026.
//

#include "JvmFunctions.h"
#include <unordered_map>

static std::unordered_map<const char*, InstanceKlass*> klasses;

InstanceKlass* JvmFunctions::findKlass17JVM(const char* name)
{
    if (klasses[name])
    {
        return klasses[name];
    }
    for (ClassLoaderData* classLoaderData = ClassLoaderData::_head();classLoaderData;classLoaderData = classLoaderData->_next())
    {
        InstanceKlass* klass = classLoaderData->_klasses();
        while (klass)
        {
            if (strcmp(name,klass ->_name() ->as_C_string()) == 0)
            {
                klasses[name] = klass;
                return klass;
            }
            klass = klass->_next();
        }
    }
    return nullptr;
}
