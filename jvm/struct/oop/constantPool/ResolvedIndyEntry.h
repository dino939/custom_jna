//
// Created by prone on 04.08.2026.
//

#ifndef CUSTOM_JNA_RESOLVEDINDYENTRY_H
#define CUSTOM_JNA_RESOLVEDINDYENTRY_H

#include "../../../types.h"

class ResolvedIndyEntry {
public:
    // Stub storage so Array<ResolvedIndyEntry> lays out like HotSpot:
    // _data lives at array+8 and each element is 16 bytes (alignof 8),
    // with _cpool_index (u2) at offset 0x0a. Field values are read through
    // the VMStruct-derived accessors using `this`.
    alignas(8) char _pad[16];

    u2 constant_pool_index() const;
    u2 resolved_references_index() const;
    bool has_appendix() const;
    bool is_resolved() const;
    bool resolution_failed() const;
    Method* method() const;

    bool operator==(const ResolvedIndyEntry& other) const { return this == &other; }
    bool operator!=(const ResolvedIndyEntry& other) const { return this != &other; }
};

#endif //CUSTOM_JNA_RESOLVEDINDYENTRY_H
