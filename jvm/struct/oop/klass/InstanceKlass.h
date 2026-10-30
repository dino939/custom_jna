//
// Created by prone on 17.03.2026.
//


#ifndef CUSTOM_JNA_INSTANCEKLASS_H
#define CUSTOM_JNA_INSTANCEKLASS_H
#include "../../../types.h"
#include "../handle/OopHandle.h"
#include "../symbol/Symbol.h"
#include "RecordComponent.h"

class ClassLoaderData;
class ConstantPool;
class AccessFlags;
class Annotations;
class Method;
class InstanceKlass

{
public:
    [[nodiscard]] Symbol* name() const;
    [[nodiscard]] InstanceKlass* next();
    [[nodiscard]] InstanceKlass* super();
    [[nodiscard]] InstanceKlass* subklass();
    [[nodiscard]] ClassLoaderData* loader();
    [[nodiscard]] OopHandle java_mirror();
    [[nodiscard]] AccessFlags access_flags();
    void set_access_flags(AccessFlags flags);
    [[nodiscard]] Array<u2>* fields();
    [[nodiscard]] u2 java_fields_count();
    [[nodiscard]] Array<u1>* fieldinfo_stream();
    [[nodiscard]] Annotations* annotations();
    [[nodiscard]] Array<Method*>* methods();
    [[nodiscard]] Array<int>* method_ordering();
    [[nodiscard]] ConstantPool* constants();
    void set_constant_pool(ConstantPool* pool);
    // InstanceKlass::_misc_flags, bit 0 = _misc_rewritten. Owned by the class
    // (not the Method); copy_bytecodes reads it as `method_holder()+308 & 1`
    // (decompiled JvmtiClassFileReconstituter::copy_bytecodes in jvm.dll 21.0.x).
    [[nodiscard]] bool is_rewritten() const;
    [[nodiscard]] Array<InstanceKlass*>* local_interfaces();
    Array<jushort>* inner_classes() const;

    const char* source_debug_extension() const;
    u2 nest_host_index() const;
    Array<u2>* nest_members() const;
    Array<u2>* permitted_subclasses() const;
    Array<RecordComponent*>* record_components() const;
    u2 enclosing_method_class_index() const;
    u2 enclosing_method_method_index() const;

    enum InnerClassAttributeOffset {
        // From http://mirror.eng/products/jdk/1.1/docs/guide/innerclasses/spec/innerclasses.doc10.html#18814
        inner_class_inner_class_info_offset = 0,
        inner_class_outer_class_info_offset = 1,
        inner_class_inner_name_offset = 2,
        inner_class_access_flags_offset = 3,
        inner_class_next_offset = 4
      };

    enum EnclosingMethodAttributeOffset {
        enclosing_method_class_index_offset = 0,
        enclosing_method_method_index_offset = 1,
        enclosing_method_attribute_size = 2
      };
};


#endif //CUSTOM_JNA_INSTANCEKLASS_H
