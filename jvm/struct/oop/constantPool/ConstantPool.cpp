//
// Created by prone on 21.03.2026.
//

#include "ConstantPool.h"
#include "../../../offset/OffsetExtractor.h"
#include "../../../struct/oop/array/Array.h"

inline int extract_low_short_from_int(long x) {
    return x & 0xffff;
}

inline int extract_high_short_from_int(long x) {
    return (x >> 16) & 0xffff;
}


Array<u1> *ConstantPool::tags() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_tags")->offset;
    return *reinterpret_cast<Array<u1> **>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

ConstantPoolCache *ConstantPool::cache() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_cache")->offset;
    return *reinterpret_cast<ConstantPoolCache **>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPool::set_cache(ConstantPoolCache *cache) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_cache")->offset;
    *reinterpret_cast<ConstantPoolCache **>(reinterpret_cast<uintptr_t>(this) + nextOffset) = cache;
}

InstanceKlass *ConstantPool::pool_holder() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_pool_holder")->
            offset;
    return *reinterpret_cast<InstanceKlass **>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

Array<u2> *ConstantPool::operands() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_operands")->offset;
    return *reinterpret_cast<Array<u2> **>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

Array<InstanceKlass *> *ConstantPool::resolved_klasses() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_resolved_klasses")->
            offset;
    return *reinterpret_cast<Array<InstanceKlass *> **>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}


jint ConstantPool::length() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_length")->offset;
    return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

u2 ConstantPool::minor_version() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_minor_version")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPool::set_minor_version(u2 minor_version) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_minor_version")->
            offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset) = minor_version;
}

u2 ConstantPool::major_version() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_major_version")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPool::set_major_version(u2 major_version) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_major_version")->
            offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset) = major_version;
}


u2 ConstantPool::generic_signature_index() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry(
        "ConstantPool", "_generic_signature_index")->offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPool::set_generic_signature_index(u2 generic_signature_index) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry(
        "ConstantPool", "_generic_signature_index")->offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset) = generic_signature_index;
}

u2 ConstantPool::source_file_name_index() {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry(
        "ConstantPool", "_source_file_name_index")->offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset);
}

void ConstantPool::set_source_file_name_index(u2 source_file_name_index) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry(
        "ConstantPool", "_source_file_name_index")->offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + nextOffset) = source_file_name_index;
}

intptr_t *ConstantPool::base() const {
    static offset cpSize = OffsetExtractor::getInstance()->findVMTypeEntry("ConstantPool")->size;
    return (intptr_t *) (((char *) this) + cpSize);
}

bool ConstantPool::is_within_bounds(int index) {
    return 0 <= index && index < length();
}

intptr_t *ConstantPool::obj_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return &base()[which];
}

jint *ConstantPool::int_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return (jint *) &base()[which];
}

jfloat *ConstantPool::float_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return (jfloat *) &base()[which];
}

jlong *ConstantPool::long_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return (jlong *) &base()[which];
}

jdouble *ConstantPool::double_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return (jdouble *) &base()[which];
}

Symbol **ConstantPool::symbol_at_addr(int which) {
    assert(is_within_bounds(which), "index out of bounds");
    return (Symbol **) &base()[which];
}

jint ConstantPool::string_index_at(int which) {
    //  assert(tag_at(which).is_string_index(), "Corrupted constant pool");
    return *int_at_addr(which);
}

int ConstantPool::impl_klass_ref_index_at(int which) {
    long ref_idex = *int_at_addr(which);
    return extract_low_short_from_int(ref_idex);
}

void ConstantPool::set_symbol_at(Symbol *sym, int which) {
    *symbol_at_addr(which) = sym;
}

Symbol *ConstantPool::symbol_at(int which) {
    return *symbol_at_addr(which);
}

int ConstantPool::klass_slot_at(int which) {
    long value = *int_at_addr(which);
    int name_index = extract_high_short_from_int(value);
    return name_index;
}

Symbol *ConstantPool::klass_name_at(int which) {
    return symbol_at(klass_slot_at(which));
}

int ConstantPool::uncached_klass_ref_index_at(int which) { return impl_klass_ref_index_at(which); }

int ConstantPool::impl_name_and_type_ref_index_at(int which) {
    long ref_index = *int_at_addr(which);
    return extract_high_short_from_int(ref_index);
}

int ConstantPool::uncached_name_and_type_ref_index_at(int which) { return impl_name_and_type_ref_index_at(which); }

long ConstantPool::name_and_type_at(int which) {
    return *int_at_addr(which);
}

int ConstantPool::method_type_index_at(int which) {
    return *int_at_addr(which);
}

int ConstantPool::name_ref_index_at(int which_nt) {
    long ref_index = name_and_type_at(which_nt);
    return extract_low_short_from_int(ref_index);
}


int ConstantPool::signature_ref_index_at(int which_nt) {
    long ref_index = name_and_type_at(which_nt);
    return extract_high_short_from_int(ref_index);
}

int ConstantPool::klass_index_at(int which) {
    return *int_at_addr(which);
}

int ConstantPool::method_handle_ref_kind_at(int which) {
    return extract_low_short_from_int(*int_at_addr(which));
}

int ConstantPool::method_handle_index_at(int which) {
    return extract_high_short_from_int(*int_at_addr(which));
}

void ConstantPool::set_length(int length) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_length")->offset;
    *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + nextOffset) = length;
}

void ConstantPool::set_tags(Array<u1> *tags) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_tags")->offset;
    *reinterpret_cast<Array<u1> **>(reinterpret_cast<uintptr_t>(this) + nextOffset) = tags;
}

void ConstantPool::set_operands(Array<u2> *operands) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_operands")->offset;
    *reinterpret_cast<Array<u2> **>(reinterpret_cast<uintptr_t>(this) + nextOffset) = operands;
}

void ConstantPool::set_resolved_klasses(Array<InstanceKlass *> *rk) {
    static offset nextOffset = OffsetExtractor::getInstance()->findVMStructEntry("ConstantPool", "_resolved_klasses")->
            offset;
    *reinterpret_cast<Array<InstanceKlass *> **>(reinterpret_cast<uintptr_t>(this) + nextOffset) = rk;
}

void ConstantPool::tag_at_put(int which, u1 tag) {
    tags()->at_put(which, tag);
}

void ConstantPool::symbol_at_put(int which, Symbol *s) {
    *symbol_at_addr(which) = s;
}

void ConstantPool::int_at_put(int which, jint i) {
    *int_at_addr(which) = i;
}

void ConstantPool::long_at_put(int which, jlong l) {
    *long_at_addr(which) = l;
}

void ConstantPool::float_at_put(int which, jfloat f) {
    *float_at_addr(which) = f;
}

void ConstantPool::double_at_put(int which, jdouble d) {
    *double_at_addr(which) = d;
}

void ConstantPool::unresolved_klass_at_put(int which, int name_index, int resolved_klass_index) {
    // HotSpot: high 16 bits = name_index, low 16 bits = resolved_klass_index
    *int_at_addr(which) = ((name_index & 0xffff) << 16) | (resolved_klass_index & 0xffff);
}

void ConstantPool::string_at_put(int which, Symbol *s) {
    *symbol_at_addr(which) = s;
}

void ConstantPool::field_at_put(int which, int class_index, int name_and_type_index) {
    *int_at_addr(which) = ((name_and_type_index & 0xffff) << 16) | (class_index & 0xffff);
}

void ConstantPool::method_at_put(int which, int class_index, int name_and_type_index) {
    *int_at_addr(which) = ((name_and_type_index & 0xffff) << 16) | (class_index & 0xffff);
}

void ConstantPool::interface_method_at_put(int which, int class_index, int name_and_type_index) {
    *int_at_addr(which) = ((name_and_type_index & 0xffff) << 16) | (class_index & 0xffff);
}

void ConstantPool::name_and_type_at_put(int which, int name_index, int signature_index) {
    *int_at_addr(which) = ((signature_index & 0xffff) << 16) | (name_index & 0xffff);
}

void ConstantPool::method_handle_index_at_put(int which, int ref_kind, int ref_index) {
    *int_at_addr(which) = ((ref_index & 0xffff) << 16) | (ref_kind & 0xffff);
}

void ConstantPool::method_type_index_at_put(int which, int ref_index) {
    *int_at_addr(which) = ref_index;
}

void ConstantPool::dynamic_constant_at_put(int which, int bsms_attribute_index, int name_and_type_index) {
    *int_at_addr(which) = ((name_and_type_index & 0xffff) << 16) | (bsms_attribute_index & 0xffff);
}

void ConstantPool::invoke_dynamic_at_put(int which, int bsms_attribute_index, int name_and_type_index) {
    *int_at_addr(which) = ((name_and_type_index & 0xffff) << 16) | (bsms_attribute_index & 0xffff);
}

/*


#endif //CUSTOM_JNA_CONSTANTPOOL_H
/*
nonstatic_field(ConstantPool,                _tags,                                         Array<u1>*)                            \
nonstatic_field(ConstantPool,                _cache,                                        ConstantPoolCache*)                    \
nonstatic_field(ConstantPool,                _pool_holder,                                  InstanceKlass*)                        \
nonstatic_field(ConstantPool,                _operands,                                     Array<u2>*)                            \
nonstatic_field(ConstantPool,                _resolved_klasses,                             Array<Klass*>*)                        \
nonstatic_field(ConstantPool,                _length,                                       int)                                   \
nonstatic_field(ConstantPool,                _minor_version,                                u2)                                    \
nonstatic_field(ConstantPool,                _major_version,                                u2)                                    \
nonstatic_field(ConstantPool,                _generic_signature_index,                      u2)                                    \
nonstatic_field(ConstantPool,                _source_file_name_index,                       u2)
#1#*/
