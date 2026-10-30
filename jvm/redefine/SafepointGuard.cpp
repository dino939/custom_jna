//
// Created by prone on 05.08.2026.
//

#include "SafepointGuard.h"

#include <windows.h>

#include "../pattern/patterns.h"
#include "../../utils/scanner.hpp"

namespace redefine {

namespace {

// Static offsets (RVAs) into jvm.dll for the two global words we touch.
// Derived from IDA: VMThread::_cur_vm_operation @ 0x180CF5DC8 -> RVA 0xCF5DC8,
//                     SafepointSynchronize::_state   @ 0x180CD1F10 -> RVA 0xCD1F10.
// Safe on this fixed build (ms-openjdk 21.0.12 jvm.dll), ASLR-independent.
constexpr size_t kCurVmOperationRva = 0xCF5DC8;
constexpr size_t kSafepointStateRva = 0xCD1F10;

// Simulated value returned by VM_Operation::type(). 42 == "RedefineClasses"
// in VM_Operation::_names[]; it is used as an index into _op_count[] / for the
// logging name, so it must be a real in-bounds enum.
constexpr uint8_t kFakeTypeCode[] = {0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3}; // mov eax,42; ret
constexpr uint8_t kStubZeroCode[] = {0x33, 0xC0, 0xC3};                  // xor eax,eax; ret

uint8_t *jvm_base() {
    static uint8_t *base = reinterpret_cast<uint8_t *>(GetModuleHandleA("jvm.dll"));
    return base;
}

} // namespace

bool SafepointGuard::prepare() {
    uint8_t *base = jvm_base();
    if (!base) return false;

    // begin()/end() are resolved by their (verified unique) byte patterns.
    _begin = reinterpret_cast<BeginFn>(scanner::scan(base, kSafepointBeginPattern));
    _end = reinterpret_cast<EndFn>(scanner::scan(base, kSafepointEndPattern));
    _cur_vm_operation = reinterpret_cast<void **>(base + kCurVmOperationRva);
    _state = base + kSafepointStateRva;
    if (!_begin || !_end || !_cur_vm_operation || !_state) return false;

    // Never storm into a safepoint that is already in progress (a real GC). In
    // that state (_state != 0) HotSpot's version has a debug-only assert; the
    // product build has none, so double-entry would corrupt the counter/lock.
    // Fast-fail here; begin() also serializes on Threads_lock with the VM thread.
    if (*_state != 0) return false;

    // --- Build the fake VM_Operation -------------------------------
    // A single RWX page holds the two stubs. Writing then flushing the I-cache
    // makes them ready to be called as virtual functions.
    _stub_page = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!_stub_page) return false;
    uint8_t *type_stub = static_cast<uint8_t *>(_stub_page);
    uint8_t *zero_stub = type_stub + 0x10;
    memcpy(type_stub, kFakeTypeCode, sizeof(kFakeTypeCode));
    memcpy(zero_stub, kStubZeroCode, sizeof(kStubZeroCode));
    FlushInstructionCache(GetCurrentProcess(), _stub_page, 0x1000);

    // vtable: 8 function-pointer slots (begin() reads slot 3 = +24 for type(),
    // slot 5 = +40 for a bool). Fill every slot with the zero stub so any
    // accidental call is a safe no-op; type slot returns the fake enum.
    constexpr size_t kSlots = 8;
    _fake_vtable = VirtualAlloc(nullptr, kSlots * sizeof(void *), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!_fake_vtable) return false;
    void **vt = static_cast<void **>(_fake_vtable);
    for (size_t i = 0; i < kSlots; i++) vt[i] = zero_stub;
    vt[3] = type_stub; // type() -> 42 ("RedefineClasses")
    vt[5] = zero_stub; // allow_nested / cleanup bool -> false

    // The object begin() dereferences: its [0] is the vtable pointer.
    _fake_obj = VirtualAlloc(nullptr, 0x10, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!_fake_obj) return false;
    *static_cast<void **>(_fake_obj) = _fake_vtable;

    // --- Take the safepoint ----------------------------------------
    _old_cur_vm_op = *_cur_vm_operation;
    *_cur_vm_operation = _fake_obj;
    _begin(); // state -> 1 (synchronizing) -> 2 (synchronized): all threads parked
    return true;
}

void SafepointGuard::cleanup() {
    if (_active && _end) {
        _end(); // state -> 0, disarm barrier, release Threads_lock
    }
    if (_active && _cur_vm_operation) {
        // Only restore the global we actually overwrote.
        *_cur_vm_operation = _old_cur_vm_op;
    }
    if (_stub_page) VirtualFree(_stub_page, 0, MEM_RELEASE);
    if (_fake_vtable) VirtualFree(_fake_vtable, 0, MEM_RELEASE);
    if (_fake_obj) VirtualFree(_fake_obj, 0, MEM_RELEASE);
}

SafepointGuard::SafepointGuard() {
    _active = prepare();
}

SafepointGuard::~SafepointGuard() {
    cleanup();
}

} // namespace redefine