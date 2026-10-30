//
// Created by prone on 17.03.2026.
//

#include "InstanceKlass.h"
#include "../../../struct/oop/symbol/Symbol.h"
#include "../../../utils/fields/Fieldinfo.hpp"
#include "../../../offset/OffsetExtractor.h"


Symbol* InstanceKlass::name() const
{
    static offset nameKlassOffset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_name")->offset;
    return *reinterpret_cast<Symbol**>(reinterpret_cast<uintptr_t>(this) + nameKlassOffset);
}

InstanceKlass* InstanceKlass::next()
{
    static offset next_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_next_link")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + next_offset);
}

InstanceKlass* InstanceKlass::super()
{
    static offset next_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_super")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + next_offset);
}

InstanceKlass* InstanceKlass::subklass()
{
    static offset next_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_subklass")->offset;
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(this) + next_offset);
}

ClassLoaderData* InstanceKlass::loader()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_class_loader_data")->offset;
    return *reinterpret_cast<ClassLoaderData**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

OopHandle InstanceKlass::java_mirror()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_java_mirror")->offset;
    return *reinterpret_cast<OopHandle*>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

AccessFlags InstanceKlass::access_flags()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_access_flags")->offset;
    return *reinterpret_cast<AccessFlags*>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void InstanceKlass::set_access_flags(AccessFlags access_flags)
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Klass", "_access_flags")->offset;
    *reinterpret_cast<AccessFlags*>(reinterpret_cast<uintptr_t>(this) + loader_offset) = access_flags;
}

Array<u2>* InstanceKlass::fields()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_fields")->offset;
    //specific for 17 jvm
    if (loader_offset == -1)return nullptr;
    return *reinterpret_cast<Array<u2>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u2 InstanceKlass::java_fields_count()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_java_fields_count")->offset;
    //specific for 17 jvm
    if (loader_offset == -1)return 0;
    return *reinterpret_cast<u2*>(reinterpret_cast<uintptr_t>(this) + loader_offset);

}
Array<u1>* InstanceKlass::fieldinfo_stream()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_fieldinfo_stream")->offset;
    //specific for 21 jvm
    if (loader_offset == -1)return nullptr;
    return *reinterpret_cast<Array<u1>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Annotations* InstanceKlass::annotations()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_annotations")->offset;
    return *reinterpret_cast<Annotations**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<Method*>* InstanceKlass::methods()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_methods")->offset;
    return *reinterpret_cast<Array<Method*>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<int>* InstanceKlass::method_ordering()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_method_ordering")->offset;
    return *reinterpret_cast<Array<int>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

ConstantPool* InstanceKlass::constants()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_constants")->offset;
    return *reinterpret_cast<ConstantPool**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void InstanceKlass::set_constant_pool(ConstantPool *pool) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_constants")->
            offset;
    *reinterpret_cast<ConstantPool **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = pool;
}

// InstanceKlass::_misc_flags, bit 0 = _misc_rewritten. Live offset 308 in
// jvm.dll 21.0.x (matches the decompiled
// JvmtiClassFileReconstituter::copy_bytecodes: `method_holder()+308 & 1`).
static constexpr size_t kInstanceKlassMiscFlagsOffset = 308;

bool InstanceKlass::is_rewritten() const {
    const uint8_t flags = *reinterpret_cast<const uint8_t *>(
            reinterpret_cast<uintptr_t>(this) + kInstanceKlassMiscFlagsOffset);
    return (flags & 1) != 0;
}

Array<InstanceKlass*>* InstanceKlass::local_interfaces()
{
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_local_interfaces")->offset;
    return *reinterpret_cast<Array<InstanceKlass*>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<jushort> * InstanceKlass::inner_classes() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_inner_classes")->offset;
    return *reinterpret_cast<Array<jushort>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

const char *InstanceKlass::source_debug_extension() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_source_debug_extension")->offset;
    if (loader_offset == -1) return nullptr;
    return *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u2 InstanceKlass::nest_host_index() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_nest_host_index")->offset;
    if (loader_offset == -1) return 0;
    return *reinterpret_cast<u2*>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<u2> *InstanceKlass::nest_members() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_nest_members")->offset;
    if (loader_offset == -1) return nullptr;
    return *reinterpret_cast<Array<u2>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<u2> *InstanceKlass::permitted_subclasses() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_permitted_subclasses")->offset;
    if (loader_offset == -1) return nullptr;
    return *reinterpret_cast<Array<u2>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

Array<RecordComponent*> *InstanceKlass::record_components() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_record_components")->offset;
    if (loader_offset == -1) return nullptr;
    return *reinterpret_cast<Array<RecordComponent*>**>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u2 InstanceKlass::enclosing_method_class_index() const {
    Array<jushort> *ic = inner_classes();
    if (ic == nullptr) return 0;
    int len = ic->length();
    if (len == 0 || len % inner_class_next_offset != enclosing_method_attribute_size) return 0;
    return ic->at(len - enclosing_method_attribute_size + enclosing_method_class_index_offset);
}

u2 InstanceKlass::enclosing_method_method_index() const {
    Array<jushort> *ic = inner_classes();
    if (ic == nullptr) return 0;
    int len = ic->length();
    if (len == 0 || len % inner_class_next_offset != enclosing_method_attribute_size) return 0;
    return ic->at(len - enclosing_method_attribute_size + enclosing_method_method_index_offset);
}
