//
// Created by prone on 15.04.2026.
//

#ifndef CUSTOM_JNA_CONSTANTPOOLCACHE_H
#define CUSTOM_JNA_CONSTANTPOOLCACHE_H
#include "ConstantPool.h"
#include "ResolvedIndyEntry.h"
#include "ConstantPoolCacheEntry.h"

class OopHandle;

class ConstantPoolCache
{
    public:

    OopHandle resolved_references();
    Array<u2>* reference_map();
    jint length();
    void set_constant_pool(ConstantPool *pool);
    ConstantPool* constant_pool();

    ConstantPoolCacheEntry* entry_at(int i) const;
    Array<ResolvedIndyEntry>* resolved_indy_entries();
    ResolvedIndyEntry* resolved_indy_entry_at(int index);
};

#endif //CUSTOM_JNA_CONSTANTPOOLCACHE_H


// Rewriter::Rewriter(InstanceKlass* klass, const constantPoolHandle& cpool, Array<Method*>* methods, TRAPS)
//   : _klass(klass),
//     _pool(cpool),
//     _methods(methods),
//     _cp_map(cpool->length()),
//     _cp_cache_map(cpool->length() / 2),
//     _reference_map(cpool->length()),
//     _resolved_references_map(cpool->length() / 2),
//     _invokedynamic_references_map(cpool->length() / 2),
//     _method_handle_invokers(cpool->length()),
//     _invokedynamic_index(0)

