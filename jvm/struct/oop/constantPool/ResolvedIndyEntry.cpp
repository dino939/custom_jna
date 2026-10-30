//
// Created by prone on 04.08.2026.
//

#include "ResolvedIndyEntry.h"
#include "../../../offset/OffsetExtractor.h"

static offset get_offset(const char* field) {
    VMStructEntry* entry = OffsetExtractor::getInstance()->findVMStructEntry("ResolvedIndyEntry", field);
    if (entry == nullptr || entry->offset < 0) {
        return static_cast<offset>(-1);
    }
    return entry->offset;
}

static offset cp_index_offset() {
    static offset off = get_offset("_cpool_index");
    return off;
}

static offset ref_index_offset() {
    static offset off = get_offset("_resolved_references_index");
    return off;
}

static offset flags_offset() {
    static offset off = get_offset("_flags");
    return off;
}

static offset method_offset() {
    static offset off = get_offset("_method");
    return off;
}

u2 ResolvedIndyEntry::constant_pool_index() const {
    offset off = cp_index_offset();
    if (off == static_cast<offset>(-1)) {
        return *reinterpret_cast<const u2*>(reinterpret_cast<uintptr_t>(this));
    }
    return *reinterpret_cast<const u2*>(reinterpret_cast<uintptr_t>(this) + off);
}

u2 ResolvedIndyEntry::resolved_references_index() const {
    offset off = ref_index_offset();
    if (off == static_cast<offset>(-1)) {
        return *reinterpret_cast<const u2*>(reinterpret_cast<uintptr_t>(this) + sizeof(u2));
    }
    return *reinterpret_cast<const u2*>(reinterpret_cast<uintptr_t>(this) + off);
}

bool ResolvedIndyEntry::has_appendix() const {
    offset off = flags_offset();
    if (off == static_cast<offset>(-1)) {
        return false;
    }
    u1 flags = *reinterpret_cast<const u1*>(reinterpret_cast<uintptr_t>(this) + off);
    return (flags & 0x01) != 0; // has_appendix bit depends on layout; caller should verify
}

bool ResolvedIndyEntry::is_resolved() const {
    offset off = method_offset();
    if (off == static_cast<offset>(-1)) {
        return false;
    }
    return *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(this) + off) != nullptr;
}

bool ResolvedIndyEntry::resolution_failed() const {
    // No direct offset known; infer from flags if available.
    offset off = flags_offset();
    if (off == static_cast<offset>(-1)) {
        return false;
    }
    u1 flags = *reinterpret_cast<const u1*>(reinterpret_cast<uintptr_t>(this) + off);
    return (flags & 0x04) != 0;
}

Method* ResolvedIndyEntry::method() const {
    offset off = method_offset();
    if (off == static_cast<offset>(-1)) {
        return nullptr;
    }
    return *reinterpret_cast<Method* const*>(reinterpret_cast<uintptr_t>(this) + off);
}
