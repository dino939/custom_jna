//
// Created by prone on 04.08.2026.
//

#include "ConstantPoolCacheEntry.h"
#include "../../../offset/OffsetExtractor.h"

int ConstantPoolCacheEntry::constant_pool_index() const {
    static offset indices_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPoolCacheEntry", "_indices")->offset;
    if (indices_offset == static_cast<offset>(-1)) {
        indices_offset = 0;
    }
    intptr_t indices = *reinterpret_cast<const intptr_t*>(reinterpret_cast<uintptr_t>(this) + indices_offset);
    return static_cast<int>(static_cast<uint64_t>(indices) & 0xFFFF);
}
