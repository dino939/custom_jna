#pragma once
#include <filesystem>
#include <fstream>

#include "../../jvm/struct/oop/klass/InstanceKlass.h"
#include "../../jvm/struct/oop/symbol/SymbolHash.hpp"
#include "../../jvm/struct/oop/method/method.h"
#include "../../jvm/utils/fields/FieldInfoReader.hpp"


enum JVMAccessFlags_1 {
    JVM_ACC_PUBLIC = 0x0001,
    JVM_ACC_PRIVATE = 0x0002,
    JVM_ACC_PROTECTED = 0x0004,
    JVM_ACC_STATIC = 0x0008,
    JVM_ACC_FINAL = 0x0010,
    JVM_ACC_SYNCHRONIZED = 0x0020,
    JVM_ACC_SUPER = 0x0020,
    JVM_ACC_VOLATILE = 0x0040,
    JVM_ACC_BRIDGE = 0x0040,
    JVM_ACC_TRANSIENT = 0x0080,
    JVM_ACC_VARARGS = 0x0080,
    JVM_ACC_NATIVE = 0x0100,
    JVM_ACC_INTERFACE = 0x0200,
    JVM_ACC_ABSTRACT = 0x0400,
    JVM_ACC_STRICT = 0x0800,
    JVM_ACC_SYNTHETIC = 0x1000,
    JVM_ACC_ANNOTATION = 0x2000,
    JVM_ACC_ENUM = 0x4000,
    JVM_ACC_MODULE = 0x8000
};

#define JVM_RECOGNIZED_CLASS_MODIFIERS (JVM_ACC_PUBLIC | \
                                        JVM_ACC_FINAL | \
                                        JVM_ACC_SUPER | \
                                        JVM_ACC_INTERFACE | \
                                        JVM_ACC_ABSTRACT | \
                                        JVM_ACC_ANNOTATION | \
                                        JVM_ACC_ENUM | \
                                        JVM_ACC_SYNTHETIC)

#define JVM_RECOGNIZED_FIELD_MODIFIERS (JVM_ACC_PUBLIC | \
                                        JVM_ACC_PRIVATE | \
                                        JVM_ACC_PROTECTED | \
                                        JVM_ACC_STATIC | \
                                        JVM_ACC_FINAL | \
                                        JVM_ACC_VOLATILE | \
                                        JVM_ACC_TRANSIENT | \
                                        JVM_ACC_ENUM | \
                                        JVM_ACC_SYNTHETIC)

#define JVM_RECOGNIZED_METHOD_MODIFIERS (JVM_ACC_PUBLIC | \
                                         JVM_ACC_PRIVATE | \
                                         JVM_ACC_PROTECTED | \
                                         JVM_ACC_STATIC | \
                                         JVM_ACC_FINAL | \
                                         JVM_ACC_SYNCHRONIZED | \
                                         JVM_ACC_BRIDGE | \
                                         JVM_ACC_VARARGS | \
                                         JVM_ACC_NATIVE | \
                                         JVM_ACC_ABSTRACT | \
                                         JVM_ACC_STRICT | \
                                         JVM_ACC_SYNTHETIC)

class ClassFileReconstituter {
public:
    ClassFileReconstituter(InstanceKlass *_ik);

    ~ClassFileReconstituter();

    const std::vector<uint8_t> &bytes() const { return _bytes; }

private:
    std::ofstream stream;
    std::vector<uint8_t> _bytes;
    SymbolHash symmap;
    SymbolHash classmap;
    InstanceKlass *_ik = nullptr;


    uint16_t symbolToConstantPoolIndex(std::string) const;

    uint16_t classNameToConstantPoolIndex(std::string) const;


    void write_utf8(Symbol *);

    void write_u1(uint8_t);

    void write_u2(uint16_t);

    void write_u4(uint32_t);

    void write_u8(uint64_t);

    void write_attribute_name_index(const char *);

    void write_method_info(Method *);

    void write_code_attribute(Method *);

    // void write_exceptions_attribute(ConstMethod*);
    uint16_t line_number_table_entries(Method *);

    // uint16_t inner_classes_attribute_length(InstanceKlass*);

    uint16_t inner_classes_attribute_length(InstanceKlass *ik);

    void write_exceptions_attribute(ConstMethod *const_method);

    void write_inner_classes_attribute(InstanceKlass *ik);

    void write_enclosing_method_attribute(InstanceKlass *ik);

    void write_source_debug_extension_attribute(InstanceKlass *ik);

    void write_nest_host_attribute(InstanceKlass *ik);

    void write_nest_members_attribute(InstanceKlass *ik);

    void write_permitted_subclasses_attribute(InstanceKlass *ik);

    void write_record_attribute(InstanceKlass *ik);
};
