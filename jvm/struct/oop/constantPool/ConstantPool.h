//
// Created by prone on 21.03.2026.
//

#ifndef CUSTOM_JNA_CONSTANTPOOL_H
#define CUSTOM_JNA_CONSTANTPOOL_H
#include "../../../struct/oop/klass/InstanceKlass.h"
enum ConstantType {
    JVM_CONSTANT_Invalid = 0,
    JVM_CONSTANT_Utf8 = 1,
    JVM_CONSTANT_Unicode = 2,
    JVM_CONSTANT_Integer = 3,
    JVM_CONSTANT_Float = 4,
    JVM_CONSTANT_Long = 5,
    JVM_CONSTANT_Double = 6,
    JVM_CONSTANT_Class = 7,
    JVM_CONSTANT_String = 8,
    JVM_CONSTANT_Fieldref = 9,
    JVM_CONSTANT_Methodref = 10,
    JVM_CONSTANT_InterfaceMethodref = 11,
    JVM_CONSTANT_NameAndType = 12,
    JVM_CONSTANT_MethodHandle = 15,
    JVM_CONSTANT_MethodType = 16,
    JVM_CONSTANT_Dynamic = 17,
    JVM_CONSTANT_InvokeDynamic = 18,
    JVM_CONSTANT_Module = 19,
    JVM_CONSTANT_Package = 20,
    JVM_CONSTANT_ExternalMax = 20,

    JVM_CONSTANT_InternalMin = 100,
    JVM_CONSTANT_UnresolvedClass = 100,
    JVM_CONSTANT_ClassIndex = 101,
    JVM_CONSTANT_StringIndex = 102,
    JVM_CONSTANT_UnresolvedClassInError = 103,
    JVM_CONSTANT_MethodHandleInError = 104,
    JVM_CONSTANT_MethodTypeInError = 105,
    JVM_CONSTANT_DynamicInError = 106,
    JVM_CONSTANT_InternalMax = 106
};

class ConstantPool
{
public:
    enum
    {
        _has_preresolution = 1, // Flags
        _on_stack = 2,
        _is_shared = 4,
        _has_dynamic_constant = 8
    };
    union {
        int                _resolved_reference_length;
        int                _version;
    } _saved;


    Array<u1>* tags();
    ConstantPoolCache* cache();
    void set_cache(ConstantPoolCache* cache);
    InstanceKlass* pool_holder();
    Array<u2>* operands();
    Array<InstanceKlass*>* resolved_klasses();
    jint length();
    u2 minor_version();
    void set_minor_version(u2 minor_version);
    u2 major_version();
    void set_major_version(u2 major_version);
    u2 generic_signature_index();
    void set_generic_signature_index(u2 generic_signature_index);
    u2 source_file_name_index();
    void set_source_file_name_index(u2 source_file_name_index);
    intptr_t* base() const;
    bool is_within_bounds(int index);
    intptr_t* obj_at_addr(int which);
    jint* int_at_addr(int which);
    jfloat* float_at_addr(int which);
    jlong* long_at_addr(int which);
    jdouble* double_at_addr(int which);
    Symbol** symbol_at_addr(int which);
    jint string_index_at(int which);
    void set_symbol_at(Symbol *sym, int which);
    Symbol* symbol_at(int which);
    int impl_klass_ref_index_at(int which);
    int klass_slot_at(int which);
    Symbol* klass_name_at(int which);
    int uncached_klass_ref_index_at(int which);
    int impl_name_and_type_ref_index_at(int which);
    int uncached_name_and_type_ref_index_at(int which);
    long name_and_type_at(int which);
    int method_type_index_at(int which);
    int name_ref_index_at(int which_nt);
    int signature_ref_index_at(int which_nt);
    int klass_index_at(int which);
    int method_handle_ref_kind_at(int which);
    int method_handle_index_at(int which);

    // Setters for constant pool construction / merging
    void set_length(int length);
    void set_tags(Array<u1>* tags);
    void set_operands(Array<u2>* operands);
    void set_resolved_klasses(Array<InstanceKlass*>* rk);

    void tag_at_put(int which, u1 tag);
    void symbol_at_put(int which, Symbol* s);
    void int_at_put(int which, jint i);
    void long_at_put(int which, jlong l);
    void float_at_put(int which, jfloat f);
    void double_at_put(int which, jdouble d);

    void unresolved_klass_at_put(int which, int name_index, int resolved_klass_index);
    void string_at_put(int which, Symbol* s);

    void field_at_put(int which, int class_index, int name_and_type_index);
    void method_at_put(int which, int class_index, int name_and_type_index);
    void interface_method_at_put(int which, int class_index, int name_and_type_index);
    void name_and_type_at_put(int which, int name_index, int signature_index);
    void method_handle_index_at_put(int which, int ref_kind, int ref_index);
    void method_type_index_at_put(int which, int ref_index);
    void dynamic_constant_at_put(int which, int bsms_attribute_index, int name_and_type_index);
    void invoke_dynamic_at_put(int which, int bsms_attribute_index, int name_and_type_index);

};


#endif //CUSTOM_JNA_CONSTANTPOOL_H
