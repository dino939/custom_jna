//
// Created by prone on 17.03.2026.
//

#include "ClassLoaderData.h"

ClassLoaderData* ClassLoaderData::head = *static_cast<ClassLoaderData**>(OffsetExtractor::getInstance()->findVMStructEntry("ClassLoaderDataGraph", "_head")->address);

ClassLoaderData* ClassLoaderData::_next()
{
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ClassLoaderData", "_next")->offset;
    return *reinterpret_cast<ClassLoaderData**>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

InstanceKlass* ClassLoaderData::_klasses()
{
    static offset klassesOffset = OffsetExtractor::getInstance()->findVMStructEntry("ClassLoaderData", "_klasses")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + klassesOffset);
}
