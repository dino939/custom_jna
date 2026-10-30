//
// Created by prone on 01.07.2026.
//

#include "method.h"

#include "bytecode/Bytecode.h"

#include <windows.h>
#include <cstring>

void Method::setConstMethod(ConstMethod *const_method) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_constMethod")->offset;
    *reinterpret_cast<ConstMethod **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = const_method;
}

ConstMethod *Method::constMethod() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_constMethod")->offset;
    return *reinterpret_cast<ConstMethod **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void *Method::i2iEntry() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_i2i_entry")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void Method::setI2IEntry(void *entry) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_i2i_entry")->offset;
    *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = entry;
}

void *Method::fromInterpretedEntry() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_from_interpreted_entry")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void Method::setFromInterpretedEntry(void *entry) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_from_interpreted_entry")->offset;
    *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = entry;
}

void *Method::methodCounters() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_method_counters")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void *Method::methodData() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_method_data")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void *Method::code() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_code")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void Method::setCode(void *code) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_code")->offset;
    *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = code;
}

void *Method::adapter() const {
    // Method::_adapter is not exposed in the vmStructs, but it sits right after
    // _method_counters in the Method layout, so _method_counters + sizeof(void*)
    // reaches it. Same trick as Leaf's Method::adapter().
    static offset mc_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_method_counters")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + mc_offset + sizeof(void *));
}

void *Method::fromCompiledEntry() const {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_from_compiled_entry")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset);
}

void Method::setFromCompiledEntry(void *entry) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_from_compiled_entry")->offset;
    *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this) + loader_offset) = entry;
}

namespace {

void set_int_field(void *obj, const char *type_name, const char *field_name, uint32_t value) {
    auto *entry = OffsetExtractor::getInstance()->findVMStructEntry(type_name, field_name);
    if (entry == nullptr || obj == nullptr) {
        return;
    }
    *reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(obj) + entry->offset) = value;
}

} // namespace

void Method::disable_jit() {
    if (void *mc = methodCounters()) {
        set_int_field(mc, "MethodCounters", "_invoke_mask", 0xFFFFFFFFu);
        set_int_field(mc, "MethodCounters", "_backedge_mask", 0xFFFFFFFFu);
    }
    if (void *md = methodData()) {
        set_int_field(md, "MethodData", "_invoke_mask", 0xFFFFFFFFu);
        set_int_field(md, "MethodData", "_backedge_mask", 0xFFFFFFFFu);
    }
}

namespace {

// CompiledMethod::make_not_entrant states (nmethod::_state).
constexpr signed char kNotInstalled = -1;
constexpr signed char kInUse = 0;
constexpr signed char kNotUsed = 1;
constexpr signed char kNotEntrant = 2;

// InvocationEntryBci == -1, i.e. not an on-stack-replacement compilation.
constexpr int kInvocationEntryBci = -1;

// JDK21 AdapterHandlerEntry::_c2i_entry (after _fingerprint at 0x0 and
// _i2c_entry at 0x8). Matches Leaf's AdapterHandlerEntry::offsetC2I.
constexpr uintptr_t kAdapterC2IEntryOffset = 0x10;

void *nmethod_entry_point(void *nm) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("nmethod", "_entry_point")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(nm) + loader_offset);
}

void *nmethod_verified_entry_point(void *nm) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("nmethod", "_verified_entry_point")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(nm) + loader_offset);
}

int nmethod_entry_bci(void *nm) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("nmethod", "_entry_bci")->offset;
    return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(nm) + loader_offset);
}

signed char nmethod_state(void *nm) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("nmethod", "_state")->offset;
    return *reinterpret_cast<volatile signed char *>(reinterpret_cast<uintptr_t>(nm) + loader_offset);
}

void set_nmethod_state(void *nm, signed char state) {
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("nmethod", "_state")->offset;
    *reinterpret_cast<volatile signed char *>(reinterpret_cast<uintptr_t>(nm) + loader_offset) = state;
}

void *shared_runtime_wrong_method_stub() {
    // SharedRuntime::_wrong_method_blob is a static RuntimeStub*; its address
    // in vmStructs points at the pointer itself.
    auto *entry = OffsetExtractor::getInstance()->findVMStructEntry("SharedRuntime", "_wrong_method_blob");
    if (entry == nullptr || entry->address == nullptr) {
        return nullptr;
    }
    void *stub = *reinterpret_cast<void **>(entry->address);
    if (stub == nullptr) {
        return nullptr;
    }
    // RuntimeStub::entryPoint() == CodeBlob::_code_begin.
    static offset loader_offset = OffsetExtractor::getInstance()->findVMStructEntry("CodeBlob", "_code_begin")->offset;
    return *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(stub) + loader_offset);
}

// NativeJump::patchVerifiedEntry: 5-byte E9 rel32 jump to dest.
void patch_verified_entry(void *verified_entry, void *dest) {
    auto *code = static_cast<uint8_t *>(verified_entry);
    DWORD old_protect;
    VirtualProtect(verified_entry, 5, PAGE_EXECUTE_READWRITE, &old_protect);
    code[0] = 0xE9;
    auto rel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(dest) - (reinterpret_cast<uintptr_t>(verified_entry) + 5));
    *reinterpret_cast<int32_t *>(code + 1) = rel;
    VirtualProtect(verified_entry, 5, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), verified_entry, 5);
}

// Port of nmethod::make_not_entrant() (minus OSR-list removal, which needs
// InstanceKlass/ConstantPool access we don't have here). Future calls into the
// compiled code are redirected to the wrong-method stub, so they fall back to
// the interpreter (where the spoofed stub is installed).
void make_nmethod_not_entrant(void *nm) {
    if (nmethod_entry_bci(nm) != kInvocationEntryBci && nmethod_state(nm) <= kInUse) {
        // OSR method: no caller jumps to its verified entry point, nothing to patch.
    } else if (nmethod_state(nm) != kNotEntrant) {
        if (void *verified = nmethod_verified_entry_point(nm)) {
            if (void *stub = shared_runtime_wrong_method_stub()) {
                patch_verified_entry(verified, stub);
            }
        }
    }
    set_nmethod_state(nm, kNotEntrant);
}

} // namespace

void Method::deoptimize() {
    void *nm = code();
    if (nm == nullptr) {
        return;
    }
    make_nmethod_not_entrant(nm);
    clearCode();
}

void Method::clearCode() {
    // Port of Method::clear_code(): restored compiled entry = adapter's c2i
    // entry (falls back to the interpreter), interpreted entry = i2i stub, and
    // drop the compiled code reference.
    void *ad = adapter();
    setFromCompiledEntry(ad != nullptr
                             ? *reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(ad) + kAdapterC2IEntryOffset)
                             : nullptr);
    setFromInterpretedEntry(i2iEntry());
    setCode(nullptr);
}


ConstMethod::MethodType Method::method_type() const {
    return constMethod()->method_type();
}

bool Method::is_overpass() const {
    return method_type() == ConstMethod::OVERPASS;
}

void Method::allocate_constmethod_with_new_bytecode(std::vector<uint8_t> bytecode,
                                                    const std::vector<ExceptionTableElement> &exception_table) {
    ConstMethod *old_cm = constMethod();
    if (old_cm == nullptr) return;
    // Fast path: no exception-table change and the bytecode shrinks (or fits) --
    // the existing ConstMethod (including any exception table) can be reused.
    if (exception_table.empty() && !old_cm->has_exception_table() &&
        bytecode.size() <= old_cm->code_size()) {
        old_cm->setBytecode(bytecode);
        return;
    }

    constexpr size_t WORD_SIZE = sizeof(intptr_t);
    const auto align_up = [](size_t v, size_t a) { return (v + a - 1) & ~(a - 1); };

    const size_t header_size = []() -> size_t {
        if (auto *type_entry = OffsetExtractor::getInstance()->findVMTypeEntry("ConstMethod")) {
            return type_entry->size;
        }
        return 0;
    }();
    if (header_size == 0) return;

    const size_t old_code_size = old_cm->code_size();
    const size_t new_code_size = bytecode.size();

    auto *code_base = static_cast<u1 *>(old_cm->code_base());
    auto *old_code_end = code_base + old_code_size;
    auto *old_end = static_cast<u1 *>(old_cm->constMethod_end());
    if (code_base == nullptr || old_end < old_code_end) return;

    // End of the compressed line-number table (right after the bytecodes).
    // The local variable table (between it and the exception table) is dropped
    // on rebuild: it is debug-only metadata and its HotSpot accessor here is
    // unreliable once an exception table is added.
    u1 *ln_end = old_code_end;
    if (old_cm->has_linenumber_table() && old_code_end + 2 <= old_end) {
        u2 ln_count = *reinterpret_cast<u2 *>(old_code_end); // native u2
        size_t ln_bytes = 2 + static_cast<size_t>(ln_count) * 4;
        if (old_code_end + ln_bytes <= old_end) {
            ln_end = old_code_end + ln_bytes;
        }
    }

    // Location where the new exception-table length u2 must live (before the
    // checked_exceptions / method_parameters / generic / annotations sections).
    u1 *insert_slot = reinterpret_cast<u1 *>(old_cm->exception_table_length_addr());
    if (insert_slot == nullptr || insert_slot < ln_end) insert_slot = ln_end;
    if (insert_slot > old_end) insert_slot = old_end;
    u1 *rest_start = insert_slot + 2; // sections strictly after the exception table
    if (rest_start > old_end) rest_start = old_end;

    const size_t ln_size = ln_end - old_code_end;
    const size_t rest_size = old_end - rest_start;
    const size_t exc_elems_bytes = exception_table.size() * sizeof(ExceptionTableElement);
    const size_t exc_bytes = exception_table.empty() ? 0 : exc_elems_bytes + 2;

    const size_t metadata_size = ln_size + exc_bytes + rest_size;
    const size_t new_total_bytes = align_up(header_size + new_code_size + metadata_size, WORD_SIZE);
    const size_t metadata_start = new_total_bytes - metadata_size;
    const size_t gap = metadata_start - (header_size + new_code_size);

    constexpr size_t RESERVED_SPACE = 16384;
    size_t total_alloc = new_total_bytes + RESERVED_SPACE;
    if (total_alloc % WORD_SIZE != 0) {
        total_alloc += WORD_SIZE - (total_alloc % WORD_SIZE);
    }

    void *mem = VirtualAlloc(nullptr, total_alloc, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem == nullptr) return;

    if (header_size > 0) {
        memcpy(mem, old_cm, header_size);
    }
    ConstMethod *new_cm = static_cast<ConstMethod *>(mem);

    if (old_code_size > 0 && old_cm->code_base() != nullptr) {
        memcpy(new_cm->code_base(), old_cm->code_base(), old_code_size);
    }

    auto *meta = reinterpret_cast<u1 *>(new_cm->code_base()) + new_code_size + gap;
    if (gap > 0) {
        memset(reinterpret_cast<u1 *>(new_cm->code_base()) + new_code_size, 0, gap);
    }

    // compressed line-number table
    if (ln_size > 0) {
        memcpy(meta, old_code_end, ln_size);
        meta += ln_size;
    }
    // exception table: [ExceptionTableElement*][u2 length] (native endianness)
    if (!exception_table.empty()) {
        memcpy(meta, exception_table.data(), exc_elems_bytes);
        meta += exc_elems_bytes;
        *reinterpret_cast<u2 *>(meta) = static_cast<u2>(exception_table.size());
        meta += 2;
    }
    // sections that live after the exception table (checked exceptions, params...)
    if (rest_size > 0) {
        memcpy(meta, rest_start, rest_size);
    }

    new_cm->set_constMethod_size(static_cast<int>(new_total_bytes / WORD_SIZE));
    new_cm->set_code_size(static_cast<u2>(new_code_size));

    u4 new_flags = new_cm->flags();
    new_flags &= ~ConstMethod::_has_localvariable_table;
    if (exception_table.empty()) {
        new_flags &= ~ConstMethod::_has_exception_table;
    } else {
        new_flags |= ConstMethod::_has_exception_table;
    }
    new_cm->set_flags(new_flags);

    new_cm->setBytecode(bytecode);
    setConstMethod(new_cm);
}

AccessFlags Method::accessFlags() {
    static offset access_flags_offset = OffsetExtractor::getInstance()->findVMStructEntry("Method", "_access_flags")->
            offset;
    return *reinterpret_cast<AccessFlags *>(reinterpret_cast<uintptr_t>(this) + access_flags_offset);
}

AnnotationArray *Method::annotations() const {
    return constMethod()->method_annotations();
}

AnnotationArray *Method::parameter_annotations() const {
    return constMethod()->parameter_annotations();
}

AnnotationArray *Method::annotation_default() const {
    return constMethod()->default_annotations();
}

AnnotationArray *Method::type_annotations() const {
    return constMethod()->type_annotations();
}

namespace {

size_t constmethod_header_size() {
    static size_t size = OffsetExtractor::getInstance()->findVMTypeEntry("ConstMethod")->size;
    return size;
}

ConstMethod *allocate_custom_constmethod(ConstMethod *old_cm, const std::vector<uint8_t> &bytecode) {
    if (old_cm == nullptr) {
        return nullptr;
    }

    constexpr size_t WORD_SIZE = sizeof(intptr_t);
    const auto align_up = [](size_t v, size_t a) { return (v + a - 1) & ~(a - 1); };

    const size_t header_size = constmethod_header_size();
    const size_t old_code_size = old_cm->code_size();
    const size_t old_total_bytes = static_cast<size_t>(old_cm->constMethod_size()) * sizeof(intptr_t);
    const size_t new_code_size = bytecode.size();

    const size_t metadata_size = old_total_bytes > header_size + old_code_size
                                 ? old_total_bytes - header_size - old_code_size
                                 : 0;


    const size_t new_total_bytes = align_up(header_size + new_code_size + metadata_size, WORD_SIZE);
    const size_t metadata_start = new_total_bytes - metadata_size;
    const size_t gap = metadata_start - (header_size + new_code_size);

    constexpr size_t RESERVED_SPACE = 16384;
    size_t total_alloc = new_total_bytes + RESERVED_SPACE;
    if (total_alloc % WORD_SIZE != 0) {
        total_alloc += WORD_SIZE - (total_alloc % WORD_SIZE);
    }

    void *mem = VirtualAlloc(nullptr, total_alloc, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem == nullptr) {
        return nullptr;
    }

    memcpy(mem, old_cm, header_size);
    ConstMethod *new_cm = static_cast<ConstMethod *>(mem);

    if (old_code_size > 0 && old_cm->code_base() != nullptr) {
        memcpy(new_cm->code_base(), old_cm->code_base(), old_code_size);
    }
    if (metadata_size > 0) {
        auto *new_metadata = reinterpret_cast<uint8_t *>(new_cm->code_base()) + new_code_size + gap;
        auto *old_metadata = reinterpret_cast<uint8_t *>(old_cm->code_base()) + old_code_size;
        if (gap > 0) {
            memset(reinterpret_cast<uint8_t *>(new_cm->code_base()) + new_code_size, 0, gap);
        }
        memcpy(new_metadata, old_metadata, metadata_size);
    }

    new_cm->set_constMethod_size(static_cast<int>(new_total_bytes / WORD_SIZE));
    new_cm->set_code_size(static_cast<u2>(new_code_size));
    new_cm->setBytecode(bytecode);

    return new_cm;
}

Method *allocate_shadow_method(Method *method, ConstMethod *custom_cm) {
    static size_t method_size = OffsetExtractor::getInstance()->findVMTypeEntry("Method")->size;
    if (method_size == 0) {
        return nullptr;
    }

    void *mem = VirtualAlloc(nullptr, method_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (mem == nullptr) {
        return nullptr;
    }

    memcpy(mem, method, method_size);
    auto *shadow = static_cast<Method *>(mem);
    shadow->setConstMethod(custom_cm);
    return shadow;
}


void *build_spoof_stub(Method *shadow, void *original_entry) {
    constexpr size_t STUB_SIZE = 64;
    auto *stub = static_cast<uint8_t *>(
        VirtualAlloc(nullptr, STUB_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (stub == nullptr) {
        return nullptr;
    }

    uint8_t *p = stub;
    *p++ = 0x48;
    *p++ = 0xBB;
    const uintptr_t shadow_addr = reinterpret_cast<uintptr_t>(shadow);
    std::memcpy(p, &shadow_addr, sizeof(shadow_addr));
    p += sizeof(shadow_addr);
    *p++ = 0x48;
    *p++ = 0xB8;
    const uintptr_t entry_addr = reinterpret_cast<uintptr_t>(original_entry);
    std::memcpy(p, &entry_addr, sizeof(entry_addr));
    p += sizeof(entry_addr);
    *p++ = 0xFF;
    *p++ = 0xE0;

    FlushInstructionCache(GetCurrentProcess(), stub, STUB_SIZE);
    return stub;
}

} // namespace

void Method::spoof_constmethod(std::vector<uint8_t> real_bytecode) {
    ConstMethod *orig_cm = constMethod();
    void *orig_i2i = i2iEntry();
    if (orig_cm == nullptr || orig_i2i == nullptr) {
        return;
    }

    ConstMethod *custom_cm = allocate_custom_constmethod(orig_cm, real_bytecode);
    if (custom_cm == nullptr) {
        return;
    }

    Method *shadow = allocate_shadow_method(this, custom_cm);
    if (shadow == nullptr) {
        return;
    }

    void *stub = build_spoof_stub(shadow, orig_i2i);
    if (stub == nullptr) {
        return;
    }

    // Method::_constMethod is left untouched. Only the interpreter entry is
    // redirected to the stub, which hands the real bytecode to the interpreter.
    setI2IEntry(stub);
    setFromInterpretedEntry(stub);

    // Keep the method interpreted forever so the JIT can never take over and
    // produce a compiled path that bypasses the stub.
    disable_jit();
}

void Method::redefine(std::vector<uint8_t> bytecode, bool shadow,
                      std::vector<ExceptionTableElement> exception_table) {
    if (shadow) {
        // Hide the new bytecode behind the i2i stub; the original ConstMethod
        // stays visible (anti-dump). The stub + disabled JIT guarantee the
        // interpreter always runs the new bytecode.
        spoof_constmethod(std::move(bytecode));
        return;
    }

    // Full replacement. First deoptimize so no live nmethod points at the old
    // body, then swap in a new ConstMethod carrying the new bytecode.
    deoptimize();
    allocate_constmethod_with_new_bytecode(std::move(bytecode), exception_table);
}
