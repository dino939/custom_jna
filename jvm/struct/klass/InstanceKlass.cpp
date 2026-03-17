//
// Created by prone on 17.03.2026.
//

#include "InstanceKlass.h"
#include "../../struct/symbol/Symbol.h"
#include "../../offset/offsetExtractor.h"



Symbol* InstanceKlass::_name() const
{
    static offset nameKlassOffset = offsetExtractor::getInstance()->findVMStructEntry("Klass", "_name")->offset;
    return *reinterpret_cast<Symbol**>(reinterpret_cast<uintptr_t>(this) + nameKlassOffset);
}

InstanceKlass* InstanceKlass::_next()
{
    static offset next_offset = offsetExtractor::getInstance()->findVMStructEntry("Klass","_next_link")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + next_offset);
}




