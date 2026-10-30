//
// Created by prone on 01.07.2026.
//

#ifndef XDANTIDUMPMEM_CONSTMETHOD_H
#define XDANTIDUMPMEM_CONSTMETHOD_H
#include "../../../types.h"
#include "../annotation/Annotations.h"
#include "../../../offset/OffsetExtractor.h"
#include "../symbol/Symbol.h"

#include "methodParametersElement.h"
#include "localVariableTableElement.h"
#include "checkedExceptionElement.h"
#include "exceptionTableElement.h"

class ConstMethod
{
public:
    enum
    {
        _has_linenumber_table = 0x0001,
        _has_checked_exceptions = 0x0002,
        _has_localvariable_table = 0x0004,
        _has_exception_table = 0x0008,
        _has_generic_signature = 0x0010,
        _has_method_parameters = 0x0020,
        _is_overpass = 0x0040,
        _has_method_annotations = 0x0080,
        _has_parameter_annotations = 0x0100,
        _has_type_annotations = 0x0200,
        _has_default_annotations = 0x0400
    };

    typedef enum { NORMAL, OVERPASS } MethodType;

    u4 flags() const;

    void set_flags(u4 value);

    bool has_linenumber_table() const
    {
        return (flags() & _has_linenumber_table) != 0;
    }

    bool has_method_parameters() const
    {
        return (flags() & _has_method_parameters) != 0;
    }

    bool has_checked_exceptions() const
    {
        return (flags() & _has_checked_exceptions) != 0;
    }

    bool has_generic_signature() const
    {
        return (flags() & _has_generic_signature) != 0;
    }

    bool has_method_annotations() const
    {
        return (flags() & _has_method_annotations) != 0;
    }

    bool has_parameter_annotations() const
    {
        return (flags() & _has_parameter_annotations) != 0;
    }

    bool has_type_annotations() const
    {
        return (flags() & _has_type_annotations) != 0;
    }

    bool has_default_annotations() const
    {
        return (flags() & _has_default_annotations) != 0;
    }

    bool has_exception_handler() const
    {
        return (flags() & _has_exception_table) != 0;
    }

    bool has_localvariable_table() const
    {
        return (flags() & _has_localvariable_table) != 0;
    }

    bool has_exception_table() const {
        return (flags() & _has_exception_table) != 0;
    }


    MethodType method_type() const;
    void set_constMethod_size(int size);
    int constMethod_size() const;

    u1* constMethod_end() const;

    u2* last_u2_element() const;

    void setBytecode(const std::vector<uint8_t> & bytecode);

    std::vector<uint8_t> getBytecode();

    u2* generic_signature_index_addr() const;

    u2 generic_signature_index() const;

    AnnotationArray** method_annotations_addr() const;

    AnnotationArray* method_annotations() const;

    AnnotationArray** parameter_annotations_addr() const;

    AnnotationArray* parameter_annotations() const;

    AnnotationArray** type_annotations_addr() const;

    AnnotationArray* type_annotations() const;

    AnnotationArray** default_annotations_addr() const;

    AnnotationArray* default_annotations() const;

    ConstantPool* constant_pool() const;

    void set_constant_pool(ConstantPool* pool);

    u2 name_index() const;

    u2 signature_index() const;
    void  set_code_size(const u2 size);
    u2 code_size() const;

    void* code_end() const;
    void* code_base() const;

    u2* checked_exceptions_length_addr() const;
    u2* method_parameters_length_addr()  const;
    u2* localvariable_table_length_addr() const;
    u2* exception_table_length_addr() const;

    CheckedExceptionElement* checked_exceptions_start() const;
    MethodParametersElement* method_parameters_start() const;
    LocalVariableTableElement* localvariable_table_start() const;
    ExceptionTableElement* exception_table_start() const;


    u2 localvariable_table_length() const;
    u2 exception_table_length() const;
    u2 checked_exceptions_length() const;

    u_char* compressed_linenumber_table() const;

    Array<u1>* _stack_map_data() const;
    bool has_stackmap_table() const ;
    u2 _max_stack() const;
    u2 _max_locals() const;
    void set_max_stack(u2 value);
    void set_max_locals(u2 value);


};


#endif //XDANTIDUMPMEM_CONSTMETHOD_H
