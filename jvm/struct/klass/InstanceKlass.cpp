//
// Created by prone on 17.03.2026.
//

#include "InstanceKlass.h"
#include "../../struct/symbol/Symbol.h"
#include "../../offset/OffsetExtractor.h"



Symbol* InstanceKlass::name() const
{
    static offset nameKlassOffset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_name")->offset;
    return *reinterpret_cast<Symbol**>(reinterpret_cast<uintptr_t>(this) + nameKlassOffset);
}

InstanceKlass* InstanceKlass::next()
{
    static offset next_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass","_next_link")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + next_offset);
}




