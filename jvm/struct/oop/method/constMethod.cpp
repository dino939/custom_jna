//
// Created by prone on 01.07.2026.
//

#include "constMethod.h"
#include "../annotation/Annotations.h"
#include "../constantPool/ConstantPool.h"
#include "bytecode/Bytecode.h"

u4 ConstMethod::flags() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_flags._flags")->
            offset;
    return *reinterpret_cast<u4 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void ConstMethod::set_flags(const u4 value) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_flags._flags")->
            offset;
    *reinterpret_cast<u4 *>(reinterpret_cast<uintptr_t>(this) + loader_offset) = value;
}

ConstMethod::MethodType ConstMethod::method_type() const {
    return ((flags() & _is_overpass) == 0) ? NORMAL : OVERPASS;
}

void ConstMethod::set_constMethod_size(const int size) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_constMethod_size")
            ->
            offset;
    *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + loader_offset) = size;
}

int ConstMethod::constMethod_size() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_constMethod_size")
            ->
            offset;
    return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u1 *ConstMethod::constMethod_end() const {
    // _constMethod_size хранится в СЛОВАХ (intptr_t = 8 байт на x64),
    // как в HotSpot: (intptr_t*)this + _constMethod_size.
    return reinterpret_cast<u1 *>(reinterpret_cast<intptr_t *>(const_cast<ConstMethod *>(this)) + constMethod_size());
}

u2 *ConstMethod::last_u2_element() const {
    int offset = 0;
    if (has_method_annotations()) offset++;
    if (has_parameter_annotations()) offset++;
    if (has_type_annotations()) offset++;
    if (has_default_annotations()) offset++;
    return (u2 *) ((AnnotationArray **) constMethod_end() - offset) - 1;
}

void ConstMethod::setBytecode(const std::vector<uint8_t> &bytecode) {
    const int bytecode_size = code_size();
    if (bytecode_size < bytecode.size()) {
        return;
    }

    memcpy(code_base(), bytecode.data(), bytecode.size());
    memset(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(code_base()) + bytecode.size()),
           static_cast<uint8_t>(Bytecodes::_nop), bytecode_size - bytecode.size());
}

std::vector<uint8_t> ConstMethod::getBytecode() {
    const int bytecode_size = code_size();
    std::vector<uint8_t> bytecode(bytecode_size);
    memcpy(bytecode.data(), code_base(), bytecode_size);
    return bytecode;
}

u2 *ConstMethod::generic_signature_index_addr() const {
    assert(has_generic_signature(), "called only if generic signature exists");
    return last_u2_element();
}

u2 ConstMethod::generic_signature_index() const {
    if (has_generic_signature()) {
        return *generic_signature_index_addr();
    }
    return 0;
}

AnnotationArray **ConstMethod::method_annotations_addr() const {
    return (AnnotationArray **) constMethod_end() - 1;
}

AnnotationArray *ConstMethod::method_annotations() const {
    return has_method_annotations() ? *(method_annotations_addr()) : NULL;
}

AnnotationArray **ConstMethod::parameter_annotations_addr() const {
    int offset = 1;

    if (has_method_annotations())
        offset++;

    return (AnnotationArray **) constMethod_end() - offset;
}

AnnotationArray *ConstMethod::parameter_annotations() const {
    return has_parameter_annotations() ? *(parameter_annotations_addr()) : NULL;
}

AnnotationArray **ConstMethod::type_annotations_addr() const {
    int offset = 1;

    if (has_method_annotations())
        offset++;

    if (has_parameter_annotations())
        offset++;

    return (AnnotationArray **) constMethod_end() - offset;
}

AnnotationArray *ConstMethod::type_annotations() const {
    return has_type_annotations() ? *(type_annotations_addr()) : NULL;
}

AnnotationArray **ConstMethod::default_annotations_addr() const {
    int offset = 1;

    if (has_method_annotations())
        offset++;

    if (has_parameter_annotations())
        offset++;

    if (has_type_annotations())
        offset++;

    return (AnnotationArray **) constMethod_end() - offset;
}

AnnotationArray *ConstMethod::default_annotations() const {
    return has_default_annotations() ? *(default_annotations_addr()) : NULL;
}

ConstantPool *ConstMethod::constant_pool() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_constants")->
            offset;
    return *reinterpret_cast<ConstantPool **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void ConstMethod::set_constant_pool(ConstantPool *pool) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_constants")->
            offset;
    *reinterpret_cast<ConstantPool **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = pool;
}

u2 ConstMethod::name_index() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_name_index")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u2 ConstMethod::signature_index() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_signature_index")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void ConstMethod::set_code_size(const u2 size) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_code_size")->
            offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset) = size;
}

u2 ConstMethod::code_size() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_code_size")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void *ConstMethod::code_end() const {
    auto a = (char *) code_base() + code_size();
    if (!a) return nullptr;
    return a;
}


void *ConstMethod::code_base() const {
    static auto size = OffsetExtractor::getInstance()->findVMTypeEntry("ConstMethod")->size;

    auto codeb = (void *) ((char *) this + size); // not tested

    if (!codeb) return nullptr;

    return codeb;
}

u2 *ConstMethod::checked_exceptions_length_addr() const {
    if (has_method_parameters()) {
        return (u2 *) method_parameters_start() - 1;
    } else {
        return has_generic_signature() ? (last_u2_element() - 1) : last_u2_element();
    }
}

u2 *ConstMethod::method_parameters_length_addr() const {
    return has_generic_signature() ? (last_u2_element() - 1) : last_u2_element();
}

u2 *ConstMethod::localvariable_table_length_addr() const {
    if (has_checked_exceptions()) {
        return (u2 *) checked_exceptions_start() - 1;
    } else {
        if (has_method_parameters()) {
            return (u2 *) method_parameters_start() - 2;
        } else {
            return has_generic_signature() ? (last_u2_element() - 1) : last_u2_element();
        }
    }

    return 0;
}

u2 *ConstMethod::exception_table_length_addr() const {
    if (has_checked_exceptions()) {
        return (u2 *) checked_exceptions_start() - 1;
    } else {
        if (has_method_parameters()) {
            return (u2 *) method_parameters_start() - 1;
        } else {
            return has_generic_signature() ? (last_u2_element() - 1) : last_u2_element();
        }
    }
}

u2 ConstMethod::exception_table_length() const {
    return has_exception_table() ? *(exception_table_length_addr()) : 0;
}

u2 ConstMethod::checked_exceptions_length() const {
    return has_checked_exceptions() ? *(checked_exceptions_length_addr()) : 0;
}


u_char *ConstMethod::compressed_linenumber_table() const {
    return (u_char *) code_end();
}

CheckedExceptionElement *ConstMethod::checked_exceptions_start() const {
    u2 *address = checked_exceptions_length_addr();
    u2 length = *address;

    address -= length * sizeof(CheckedExceptionElement) / sizeof(u2);
    // TODO: not worked if offset from vmStruct ( use struct ->size )

    return (CheckedExceptionElement *) address; // ?
}

MethodParametersElement *ConstMethod::method_parameters_start() const {
    u2 *address = method_parameters_length_addr();

    u2 length = *address;

    address -= length * sizeof(MethodParametersElement) / sizeof(u2); // same

    return (MethodParametersElement *) address; // ?
}

LocalVariableTableElement *ConstMethod::localvariable_table_start() const {
    u2 *address = localvariable_table_length_addr();
    u2 length = *address;

    address -= length * sizeof(LocalVariableTableElement) / sizeof(u2); // same

    return (LocalVariableTableElement *) address; // ?
}

ExceptionTableElement *ConstMethod::exception_table_start() const {
    u2 *address = exception_table_length_addr();
    u2 length = *address;
    address -= length * sizeof(ExceptionTableElement) / sizeof(u2); // same
    return (ExceptionTableElement *) address;
}

u2 ConstMethod::localvariable_table_length() const {
    return has_localvariable_table() ? *(localvariable_table_length_addr()) : 0;
}

Array<u1> *ConstMethod::_stack_map_data() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_stackmap_data")->
            offset;

    return *reinterpret_cast<Array<u1> **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

bool ConstMethod::has_stackmap_table() const {
    return this->_stack_map_data() != nullptr;
}


u2 ConstMethod::_max_stack() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_max_stack")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

u2 ConstMethod::_max_locals() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_max_locals")->
            offset;
    return *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void ConstMethod::set_max_stack(u2 value) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_max_stack")->
            offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset) = value;
}

void ConstMethod::set_max_locals(u2 value) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("ConstMethod", "_max_locals")->
            offset;
    *reinterpret_cast<u2 *>(reinterpret_cast<uintptr_t>(this) + loader_offset) = value;
}
