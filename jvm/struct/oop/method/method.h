//
// Created by prone on 01.07.2026.
//

#ifndef XDANTIDUMPMEM_METHOD_H
#define XDANTIDUMPMEM_METHOD_H
#include "constMethod.h"
#include "../annotation/Annotations.h"
#include "../../../utils/fields/Fieldinfo.hpp"


class Method {
public:
    void setConstMethod(ConstMethod *const_method);

    ConstMethod *constMethod() const;

    ConstMethod::MethodType method_type() const;

    void allocate_constmethod_with_new_bytecode(std::vector<uint8_t> bytecode,
                                                const std::vector<ExceptionTableElement> &exception_table);

    // Bytecode spoofing. Method::_constMethod is left pointing at the original
    // ConstMethod (so dumpers/scanners read the untouched bytes), but the
    // interpreter entry (_i2i_entry / _from_interpreted_entry) is redirected to
    // a per-method stub that swaps rbx (Method*) to a shadow Method whose
    // _constMethod is a custom ConstMethod containing the real bytecode.
    void spoof_constmethod(std::vector<uint8_t> real_bytecode);

    // Redefines this method's bytecode.
    //
    // shadow == true  : hides the new bytecode behind the i2i stub
    //                   (spoof_constmethod). The original ConstMethod stays
    //                   visible to dumpers/scanners; JIT is disabled for the
    //                   method so the stub is never bypassed.
    //
    // shadow == false : fully replaces the method. Any compiled code is first
    //                   deoptimized (nmethod made not-entrant + cleared), then
    //                   Method::_constMethod is swapped to a new ConstMethod
    //                   holding the new bytecode. The change is visible to
    //                   dumpers/scanners and JIT stays enabled so the new body
    //                   can be compiled on demand.
    //
    // In both modes the interpreter always executes the supplied bytecode.
    void redefine(std::vector<uint8_t> bytecode, bool shadow,
                  std::vector<ExceptionTableElement> exception_table = {});

    void *i2iEntry() const;

    void setI2IEntry(void *entry);

    void *fromInterpretedEntry() const;

    void setFromInterpretedEntry(void *entry);

    void *methodCounters() const;

    void *methodData() const;

    // Disables JIT for this method. The JDK21 interpreter fires the
    // compile-overflow check when ((counter += 2) & mask) == 0, with the mask
    // read live from MethodCounters::_invoke_mask/_backedge_mask (or the
    // MethodData equivalents). Pinning both masks to 0xFFFFFFFF makes the check
    // only fire on 32-bit counter wraparound (~2^31 invocations), so the method
    // effectively never gets compiled.
    void disable_jit();

    // JIT / deoptimization support. Ported from Leaf's Method::Deoptimize()/
    // ClearCode()/code()/adapter(), using OffsetExtractor instead of JVMToolKit.
    void *code() const;

    void setCode(void *code);

    void *adapter() const;

    void *fromCompiledEntry() const;

    void setFromCompiledEntry(void *entry);

    void deoptimize();

    void clearCode();

    bool is_overpass() const;

    AccessFlags accessFlags();

    AnnotationArray *annotations() const;

    AnnotationArray *parameter_annotations() const;

    AnnotationArray *annotation_default() const;

    AnnotationArray *type_annotations() const;

    enum Flags {
        _caller_sensitive = 1 << 0,
        _force_inline = 1 << 1,
        _dont_inline = 1 << 2,
        _hidden = 1 << 3,
        _has_injected_profile = 1 << 4,
        _intrinsic_candidate = 1 << 5,
        _reserved_stack_access = 1 << 6,
        _scoped = 1 << 7
    };
};


#endif //XDANTIDUMPMEM_METHOD_H
