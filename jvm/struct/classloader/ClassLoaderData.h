//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_CLASSLOADERDATA_H
#define CUSTOM_JNA_CLASSLOADERDATA_H
#include "../../struct/klass/InstanceKlass.h"
#include "../../offset/offsetExtractor.h"


class ClassLoaderData
{
    static ClassLoaderData* head;
public:
    static ClassLoaderData* _head()
    {
        return head;
    }

    ClassLoaderData* _next();
    InstanceKlass* _klasses();
};


#endif //CUSTOM_JNA_CLASSLOADERDATA_H