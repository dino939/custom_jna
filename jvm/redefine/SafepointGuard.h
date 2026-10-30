//
// Created by prone on 05.08.2026.
//

#ifndef CUSTOM_JNA_SAFEPOINTGUARD_H
#define CUSTOM_JNA_SAFEPOINTGUARD_H

#include <cstdint>

namespace redefine {

/**
 * RAII guard that holds the HotSpot VM's internal safepoint while in scope.
 *
 * Why this exists: this project redefines classes by mutating live InstanceKlass
 * state (constant pool swap, bytecode rewrite, ConstantPoolCache rebuild) from a
 * JNI thread, with no coordination against the other Java threads that may be
 * mid-execution of the very methods being rewritten.  HotSpot normally performs
 * redefinition at a safepoint (VM_RedefineClasses): every Java thread is parked,
 * so no thread can be running the code we are mutating.
 *
 * How it works: we invoke the real SafepointSynchronize::begin()/end() by their
 * byte patterns.  The problem is that begin() unconditionally dereferences
 * VMThread::_cur_vm_operation to learn the operation's name/type for tracing
 * (see the disassembly: `type = (*(...)(*_cur_vm_operation + 24))(_cur_vm_operation)`),
 * so it crashes when no VM_Operation is in flight (the normal, idle case for a
 * JNI-requested redefine).  We therefore fabricate a fake VM_Operation whose
 * vtable satisfies exactly the two slots begin() reads:
 *   - vtable slot +24 (index 3): type()  -> returns VM_Operation enum 42
 *     ("RedefineClasses"), a valid in-bounds index for _op_count[]/tracing.
 *   - vtable slot +40 (index 5): bool    -> returns 0 (only toggles a cleanup flag).
 * end() never re-reads _cur_vm_operation (it only uses the cached _current_type),
 * so the same fake object is safe on both sides.
 *
 * Safety / degradation: if the native functions cannot be resolved, the JVM is
 * not in _not_synchronizing state (a real safepoint is already in progress), or
 * a step fails, the guard degrades to a no-op: active()==false and redefinition
 * still runs unsynchronized rather than corrupting an in-progress safepoint.
 */
class SafepointGuard {
public:
    SafepointGuard();
    ~SafepointGuard();

    SafepointGuard(const SafepointGuard &) = delete;
    SafepointGuard &operator=(const SafepointGuard &) = delete;

    /// True only when a real safepoint was acquired and is currently held.
    bool active() const { return _active; }

private:
    using BeginFn = void (*)();
    using EndFn = void (*)();

    bool _active = false;
    void *_old_cur_vm_op = nullptr;
    void *_fake_obj = nullptr;
    void *_fake_vtable = nullptr;
    void *_stub_page = nullptr;
    uint8_t *_state = nullptr;
    void **_cur_vm_operation = nullptr;
    BeginFn _begin = nullptr;
    EndFn _end = nullptr;

    bool prepare();   // resolve natives + check _state; must be called once
    void cleanup();   // restore global + free scratch memory
};

} // namespace redefine

#endif //CUSTOM_JNA_SAFEPOINTGUARD_H