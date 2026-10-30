//
// Created by prone on 15.04.2026.
//

#include "ConstantPoolCache.h"
#include "../../../offset/OffsetExtractor.h"
#include "../../../struct/oop/array/Array.h"
#include "../../../struct/oop/handle/OopHandle.h"

OopHandle ConstantPoolCache::resolved_references()
{
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_resolved_references")->offset;
    return *reinterpret_cast<OopHandle*>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

Array<u2>* ConstantPoolCache::reference_map()
{
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_reference_map")->offset;
    return *reinterpret_cast<Array<u2>**>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

jint ConstantPoolCache::length()
{
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_length")->offset;
    return *reinterpret_cast<jint*>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPoolCache::set_constant_pool(ConstantPool *pool) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_constant_pool")->offset;
     *reinterpret_cast<ConstantPool**>(reinterpret_cast<uintptr_t>(this) + nextOffset) = pool;
}

ConstantPool* ConstantPoolCache::constant_pool()
{
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_constant_pool")->offset;
    return *reinterpret_cast<ConstantPool**>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

ConstantPoolCacheEntry* ConstantPoolCache::entry_at(int i) const {
    static offset cacheSize = OffsetExtractor::getInstance()->findVMTypeEntry("ConstantPoolCache")->size;
    uintptr_t entries = reinterpret_cast<uintptr_t>(this) + cacheSize;
    return reinterpret_cast<ConstantPoolCacheEntry*>(entries + static_cast<size_t>(i) * 4 * sizeof(intptr_t));
}

Array<ResolvedIndyEntry>* ConstantPoolCache::resolved_indy_entries() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCache", "_resolved_indy_entries")->offset;
    return *reinterpret_cast<Array<ResolvedIndyEntry>**>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

ResolvedIndyEntry* ConstantPoolCache::resolved_indy_entry_at(int index) {
    Array<ResolvedIndyEntry>* arr = resolved_indy_entries();
    if (arr == nullptr || index < 0 || index >= arr->length()) {
        return nullptr;
    }
    return arr->adr_at(index);
}

