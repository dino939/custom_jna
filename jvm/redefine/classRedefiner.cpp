//
// Created by prone on 04.08.2026.
//

#include "classRedefiner.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

#include "../../utils/scanner.hpp"
#include "SafepointGuard.h"
#include "../jna/JvmFunctions.h"
#include "../struct/oop/array/Array.h"
#include "../struct/oop/constantPool/ConstantPool.h"
#include "../struct/oop/constantPool/ConstantPoolCache.h"
#include "../struct/oop/constantPool/ConstantPoolCacheEntry.h"
#include "../struct/oop/klass/InstanceKlass.h"
#include "../struct/oop/method/bytecode/Bytecode.h"
#include "../struct/oop/method/constMethod.h"
#include "../struct/oop/method/exceptionTableElement.h"
#include "../struct/oop/method/method.h"
#include "../struct/oop/symbol/Symbol.h"
#include "../struct/thread/JavaThread.h"
#include "../offset/OffsetExtractor.h"
#include "../pattern/patterns.h"
#include "../utils/assert/assert.hpp"

namespace redefine {
    namespace {
        // ----------------------------------------------------------------------------
        // Byte-order helpers
        // ----------------------------------------------------------------------------

        inline u2 read_u2_be(const uint8_t *p) {
            return static_cast<u2>((p[0] << 8) | p[1]);
        }

        inline u4 read_u4_be(const uint8_t *p) {
            return (static_cast<u4>(p[0]) << 24) |
                   (static_cast<u4>(p[1]) << 16) |
                   (static_cast<u4>(p[2]) << 8) |
                   static_cast<u4>(p[3]);
        }

        inline void write_u2_be(uint8_t *p, u2 v) {
            p[0] = static_cast<uint8_t>(v >> 8);
            p[1] = static_cast<uint8_t>(v);
        }

        inline u2 read_u2_native(const uint8_t *p) {
            return *reinterpret_cast<const u2 *>(p);
        }

        inline u4 read_u4_native(const uint8_t *p) {
            return *reinterpret_cast<const u4 *>(p);
        }

        inline void write_u2_native(uint8_t *p, u2 v) {
            *reinterpret_cast<u2 *>(p) = v;
        }

        inline void write_u4_native(uint8_t *p, u4 v) {
            *reinterpret_cast<u4 *>(p) = v;
        }

        // ----------------------------------------------------------------------------
        // Allocation helpers (project style: VirtualAlloc outside metaspace)
        // ----------------------------------------------------------------------------

        Symbol *alloc_symbol(const uint8_t *bytes, size_t len) {
            size_t alloc_size = sizeof(Symbol) + len + 2;
            void *mem = VirtualAlloc(nullptr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!mem) return nullptr;
            Symbol *sym = new(mem) Symbol(reinterpret_cast<const char *>(bytes), len);
            // Keep the refcount above zero so JVM refcount assertions never fire.
            sym->increment_refcount();
            return sym;
        }

        template<typename T>
        Array<T> *alloc_array(int length) {
            size_t alloc_size = sizeof(Array<T>);
            if (length > 0) {
                alloc_size += static_cast<size_t>(length - 1) * sizeof(T);
            }
            alloc_size = (alloc_size + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
            void *mem = VirtualAlloc(nullptr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!mem) return nullptr;
            Array<T> *arr = new(mem) Array<T>();
            arr->set_length(length);
            if (length > 0) {
                memset(arr->data(), 0, static_cast<size_t>(length) * sizeof(T));
            }
            return arr;
        }

        ConstantPool *alloc_constant_pool(ConstantPool *old_cp, int new_length) {
            size_t header_size = 0;
            if (auto *type_entry = OffsetExtractor::getInstance()->findVMTypeEntry("ConstantPool")) {
                header_size = type_entry->size;
            }
            if (header_size == 0) {
                header_size = sizeof(ConstantPool);
            }

            size_t entry_bytes = static_cast<size_t>(new_length) * sizeof(intptr_t);
            size_t total = header_size + entry_bytes;
            total = (total + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);

            void *mem = VirtualAlloc(nullptr, total, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!mem) return nullptr;

            ConstantPool *new_cp = static_cast<ConstantPool *>(mem);
            memcpy(new_cp, old_cp, header_size);
            new_cp->set_length(new_length);
            memset(reinterpret_cast<char *>(new_cp) + header_size, 0, entry_bytes);
            return new_cp;
        }

        // ----------------------------------------------------------------------------
        // Class-file constant pool parser
        // ----------------------------------------------------------------------------

        struct ParsedCpEntry {
            u1 tag = 0;
            std::string utf8; // Utf8
            uint32_t int_value = 0; // Integer, Float
            uint64_t long_value = 0; // Long, Double
            u2 ref1 = 0; // class name, string utf8, name index, bsm index, descriptor, ref index
            u2 ref2 = 0; // name_and_type, signature, nat
            u1 ref_kind = 0; // MethodHandle
        };

        struct BootstrapMethod {
            u2 bsm_ref = 0;
            std::vector<u2> args;
        };

        struct ParsedMethod {
            std::vector<uint8_t> bytecode;
            std::vector<ExceptionTableElement> exception_table;
            u2 max_stack = 0;
            u2 max_locals = 0;
        };

        struct ParsedClassFile {
            std::vector<ParsedCpEntry> cp; // index 0 unused
            std::vector<BootstrapMethod> bootstrap_methods;
            std::vector<std::pair<std::string, ParsedMethod> > methods; // name\0sig -> method
            int minor_version = 0;
            int major_version = 0;
        };

        class ClassFileReader {
            const std::vector<uint8_t> &_data;
            size_t _pos = 0;

        public:
            explicit ClassFileReader(const std::vector<uint8_t> &data) : _data(data) {
            }

            bool check_size(size_t n) const { return _pos + n <= _data.size(); }

            u1 read_u1() {
                if (!check_size(1)) return 0;
                return _data[_pos++];
            }

            u2 read_u2() {
                if (!check_size(2)) return 0;
                u2 v = read_u2_be(&_data[_pos]);
                _pos += 2;
                return v;
            }

            u4 read_u4() {
                if (!check_size(4)) return 0;
                u4 v = read_u4_be(&_data[_pos]);
                _pos += 4;
                return v;
            }

            const uint8_t *ptr() const { return &_data[_pos]; }
            size_t pos() const { return _pos; }
            void skip(size_t n) { _pos += n; }

            std::string read_utf8(u2 len) {
                if (!check_size(len)) return {};
                std::string s(reinterpret_cast<const char *>(&_data[_pos]), len);
                _pos += len;
                return s;
            }

            bool parse(ParsedClassFile &out, std::string &error) {
                if (_data.size() < 8 || read_u4() != 0xCAFEBABE) {
                    error = "bad magic";
                    return false;
                }
                out.minor_version = read_u2();
                out.major_version = read_u2();

                u2 cp_count = read_u2();
                out.cp.resize(cp_count);

                for (u2 i = 1; i < cp_count; i++) {
                    u1 tag = read_u1();
                    out.cp[i].tag = tag;
                    switch (tag) {
                        case JVM_CONSTANT_Utf8: {
                            u2 len = read_u2();
                            out.cp[i].utf8 = read_utf8(len);
                            break;
                        }
                        case JVM_CONSTANT_Integer:
                        case JVM_CONSTANT_Float:
                            out.cp[i].int_value = read_u4();
                            break;
                        case JVM_CONSTANT_Long:
                        case JVM_CONSTANT_Double: {
                            uint32_t hi = read_u4();
                            uint32_t lo = read_u4();
                            out.cp[i].long_value = (static_cast<uint64_t>(hi) << 32) | lo;
                            if (i + 1 < cp_count) {
                                out.cp[++i].tag = 0; // trailing empty slot
                            }
                            break;
                        }
                        case JVM_CONSTANT_Class:
                        case JVM_CONSTANT_String:
                        case JVM_CONSTANT_MethodType:
                            out.cp[i].ref1 = read_u2();
                            break;
                        case JVM_CONSTANT_Fieldref:
                        case JVM_CONSTANT_Methodref:
                        case JVM_CONSTANT_InterfaceMethodref:
                        case JVM_CONSTANT_NameAndType:
                        case JVM_CONSTANT_Dynamic:
                        case JVM_CONSTANT_InvokeDynamic:
                            out.cp[i].ref1 = read_u2();
                            out.cp[i].ref2 = read_u2();
                            break;
                        case JVM_CONSTANT_MethodHandle:
                            out.cp[i].ref_kind = read_u1();
                            out.cp[i].ref1 = read_u2();
                            break;
                        case JVM_CONSTANT_Module:
                        case JVM_CONSTANT_Package:
                            out.cp[i].ref1 = read_u2();
                            break;
                        default:
                            error = "unknown constant pool tag: " + std::to_string(tag);
                            return false;
                    }
                }

                // Read access_flags, this_class, super_class, interfaces
                if (!check_size(8)) {
                    error = "truncated after constant pool";
                    return false;
                }
                skip(2 + 2 + 2);
                u2 ifaces = read_u2();
                skip(static_cast<size_t>(ifaces) * 2);

                // Skip fields
                if (!skip_members(error)) return false;

                // Parse methods (record Code bytecodes)
                if (!parse_methods(out, error)) return false;

                // Read attributes (BootstrapMethods only)
                if (!check_size(2)) {
                    error = "truncated attributes count";
                    return false;
                }
                u2 attr_count = read_u2();
                for (u2 a = 0; a < attr_count; a++) {
                    if (!check_size(6)) {
                        error = "truncated attribute header";
                        return false;
                    }
                    u2 name_idx = read_u2();
                    u4 len = read_u4();
                    if (!check_size(len)) {
                        error = "truncated attribute body";
                        return false;
                    }

                    std::string attr_name;
                    if (name_idx < out.cp.size() && out.cp[name_idx].tag == JVM_CONSTANT_Utf8) {
                        attr_name = out.cp[name_idx].utf8;
                    }

                    if (attr_name == "BootstrapMethods") {
                        if (!parse_bootstrap_methods(out, len, error)) return false;
                    } else {
                        skip(len);
                    }
                }
                return true;
            }

        private:
            bool skip_members(std::string &error) {
                if (!check_size(2)) {
                    error = "truncated members count";
                    return false;
                }
                u2 count = read_u2();
                for (u2 i = 0; i < count; i++) {
                    if (!check_size(6)) {
                        error = "truncated member header";
                        return false;
                    }
                    skip(2 + 2 + 2); // access_flags, name_index, descriptor_index
                    if (!skip_attributes(error)) return false;
                }
                return true;
            }

            bool skip_attributes(std::string &error) {
                if (!check_size(2)) {
                    error = "truncated attributes count";
                    return false;
                }
                u2 count = read_u2();
                for (u2 i = 0; i < count; i++) {
                    if (!check_size(6)) {
                        error = "truncated attribute header";
                        return false;
                    }
                    skip(2); // name_index
                    u4 len = read_u4();
                    if (!check_size(len)) {
                        error = "truncated attribute body";
                        return false;
                    }
                    skip(len);
                }
                return true;
            }

            bool parse_methods(ParsedClassFile &out, std::string &error) {
                if (!check_size(2)) {
                    error = "truncated methods count";
                    return false;
                }
                u2 count = read_u2();
                for (u2 i = 0; i < count; i++) {
                    if (!check_size(6)) {
                        error = "truncated method header";
                        return false;
                    }
                    skip(2); // access_flags
                    u2 name_idx = read_u2();
                    u2 desc_idx = read_u2();

                    std::string key;
                    if (name_idx < out.cp.size() && out.cp[name_idx].tag == JVM_CONSTANT_Utf8) {
                        key = out.cp[name_idx].utf8;
                        key.push_back('\0');
                        if (desc_idx < out.cp.size() && out.cp[desc_idx].tag == JVM_CONSTANT_Utf8) {
                            key += out.cp[desc_idx].utf8;
                        }
                    }

                    if (!check_size(2)) {
                        error = "truncated method attributes count";
                        return false;
                    }
                    u2 attr_count = read_u2();
                    bool found_code = false;
                    for (u2 a = 0; a < attr_count; a++) {
                        if (!check_size(6)) {
                            error = "truncated method attribute header";
                            return false;
                        }
                        u2 attr_name_idx = read_u2();
                        u4 len = read_u4();
                        if (!check_size(len)) {
                            error = "truncated method attribute body";
                            return false;
                        }

                        std::string attr_name;
                        if (attr_name_idx < out.cp.size() && out.cp[attr_name_idx].tag == JVM_CONSTANT_Utf8) {
                            attr_name = out.cp[attr_name_idx].utf8;
                        }

                        if (attr_name == "Code" && !found_code) {
                            if (len < 8) {
                                error = "truncated Code attribute";
                                return false;
                            }
                            size_t code_attr_start = _pos;
                            u2 max_stack = read_u2();
                            u2 max_locals = read_u2();
                            u4 code_len = read_u4();
                            if (len < 8 + code_len) {
                                error = "truncated Code attribute body";
                                return false;
                            }
                            ParsedMethod pm;
                            pm.bytecode.resize(code_len);
                            memcpy(pm.bytecode.data(), ptr(), code_len);
                            pm.max_stack = max_stack;
                            pm.max_locals = max_locals;
                            _pos += code_len;

                            // Exception table (big-endian in the class file).
                            if (len >= 8 + code_len + 2) {
                                u2 exc_len = read_u2();
                                if (len < 8 + code_len + 2 + static_cast<size_t>(exc_len) * 8) {
                                    error = "truncated Code exception table";
                                    return false;
                                }
                                pm.exception_table.resize(exc_len);
                                for (u2 k = 0; k < exc_len; k++) {
                                    ExceptionTableElement &e = pm.exception_table[k];
                                    e.start_pc = read_u2();
                                    e.end_pc = read_u2();
                                    e.handler_pc = read_u2();
                                    e.catch_type_index = read_u2();
                                }
                            }

                            out.methods.emplace_back(key, std::move(pm));
                            found_code = true;
                            _pos = code_attr_start + len;
                        } else {
                            skip(len);
                        }
                    }
                }
                return true;
            }

            bool parse_bootstrap_methods(ParsedClassFile &out, u4 len, std::string &error) {
                size_t end = _pos + len;
                if (end > _data.size()) {
                    error = "bad BootstrapMethods length";
                    return false;
                }
                if (_pos + 2 > end) {
                    error = "truncated BootstrapMethods count";
                    return false;
                }
                u2 bsm_count = read_u2();
                for (u2 i = 0; i < bsm_count; i++) {
                    if (_pos + 4 > end) {
                        error = "truncated BootstrapMethod entry";
                        return false;
                    }
                    BootstrapMethod bm;
                    bm.bsm_ref = read_u2();
                    u2 argc = read_u2();
                    if (_pos + argc * 2 > end) {
                        error = "truncated BootstrapMethod args";
                        return false;
                    }
                    bm.args.resize(argc);
                    for (u2 j = 0; j < argc; j++) {
                        bm.args[j] = read_u2();
                    }
                    out.bootstrap_methods.push_back(std::move(bm));
                }
                _pos = end;
                return true;
            }
        };

        // ----------------------------------------------------------------------------
        // Constant pool merge
        // ----------------------------------------------------------------------------

        struct BootstrapRecord {
            size_t second_offset = 0; // start index in second_part
            u2 bsm_ref = 0;
            u2 argc = 0;
            std::vector<u2> args;
        };

        struct OperandsBuilder {
            std::vector<u2> first_part;  // u4 offsets (2 u2 each), computed at build time
            std::vector<u2> second_part; // bsm record data
            std::vector<BootstrapRecord> records;
        };

        // Re-read old bsm records using HotSpot's operands layout:
        // first part = one u4 offset per bsm (2 u2: low, high), the first offset
        // equals the start of the second part (== 2 * num_bsms).  The old records
        // keep their CP indices since old entries are copied 1:1 into the merged pool.
        static int copy_old_operands(Array<u2> *old_ops, OperandsBuilder &builder) {
            if (old_ops == nullptr || old_ops->length() < 4) return 0;
            int second_start = (int)old_ops->at(0) | ((int)old_ops->at(1) << 16);
            if (second_start <= 0 || second_start > old_ops->length()) return 0;
            int num = second_start / 2;
            int count = 0;
            for (int i = 0; i < num; i++) {
                int base = (int)old_ops->at(i * 2) | ((int)old_ops->at(i * 2 + 1) << 16);
                if (base < second_start || base + 2 > old_ops->length()) break;
                BootstrapRecord rec;
                rec.second_offset = builder.second_part.size();
                rec.bsm_ref = old_ops->at(base);
                rec.argc = old_ops->at(base + 1);
                rec.args.resize(rec.argc);
                for (u2 j = 0; j < rec.argc; j++) {
                    rec.args[j] = old_ops->at(base + 2 + j);
                }
                builder.second_part.push_back(rec.bsm_ref);
                builder.second_part.push_back(rec.argc);
                for (u2 a: rec.args) builder.second_part.push_back(a);
                builder.records.push_back(std::move(rec));
                count++;
            }
            return count;
        }

        static int append_bootstrap_method(OperandsBuilder &builder, const BootstrapMethod &bm) {
            int index = static_cast<int>(builder.records.size());
            BootstrapRecord rec;
            rec.second_offset = builder.second_part.size();
            rec.bsm_ref = bm.bsm_ref;
            rec.argc = static_cast<u2>(bm.args.size());
            rec.args = bm.args;
            builder.second_part.push_back(rec.bsm_ref);
            builder.second_part.push_back(rec.argc);
            for (u2 arg: rec.args) {
                // args are CP indices already remapped to the merged pool by caller
                builder.second_part.push_back(arg);
            }
            builder.records.push_back(std::move(rec));
            return index;
        }

        static Array<u2> *build_operands_array(OperandsBuilder &builder) {
            int n = static_cast<int>(builder.records.size());
            if (n == 0) return nullptr;
            builder.first_part.clear();
            builder.first_part.reserve(static_cast<size_t>(n) * 2);
            for (int i = 0; i < n; i++) {
                int offset = n * 2 + static_cast<int>(builder.records[i].second_offset);
                builder.first_part.push_back(static_cast<u2>(offset & 0xffff));
                builder.first_part.push_back(static_cast<u2>((offset >> 16) & 0xffff));
            }
            int total = static_cast<int>(builder.first_part.size() + builder.second_part.size());
            Array<u2> *arr = alloc_array<u2>(total);
            if (!arr) return nullptr;
            int pos = 0;
            for (u2 v: builder.first_part) arr->at_put(pos++, v);
            for (u2 v: builder.second_part) arr->at_put(pos++, v);
            return arr;
        }

        struct MergeState {
            ConstantPool *old_cp = nullptr;
            ConstantPool *merged_cp = nullptr;
            ParsedClassFile *parsed = nullptr;
            std::vector<int> *index_map = nullptr; // parsed index -> merged index
            std::vector<int> *bsm_index_map = nullptr; // parsed bsm index -> merged bsm index
            std::unordered_map<std::string, Symbol *> utf8_cache; // content -> symbol
            int merged_length = 0;
            int next_rk_index = 0;
            OperandsBuilder operands;
            std::string error; // set on first append failure; checked by the caller
        };

        static int fail_append(MergeState &s, int parsed_index) {
            if (s.error.empty()) {
                s.error = "merge failed: parsed CP index " + std::to_string(parsed_index) + " (tag " +
                          std::to_string(s.parsed->cp[parsed_index].tag) + ") could not be appended to the merged pool";
            }
            return 0;
        }

        // Reuse an interned symbol with the same content instead of allocating
        // a duplicate.  HotSpot resolves classes/fields/methods by interned
        // Symbol pointer identity (SymbolTable), so a symbol created on the
        // heap (VirtualAlloc) is never found by SystemDictionary::resolve().
        //
        // Instead of walking every ClassLoaderData / InstanceKlass constant pool
        // by hand (brittle against the running JVM's layout and unsafe to
        // traverse while other threads mutate classes), we call the real
        // SymbolTable::new_symbol(name, len).  It locates the existing interned
        // Symbol or inserts a fresh one with the correct refcount.  This keeps
        // the symbol reachable from the SymbolTable and matching by identity.
        using SymbolTableNewSymbolFn = Symbol *(*)(const char *name, int len);

        // SymbolTable::new_symbol(const char*, int)  (ms-openjdk 21.0.12, jvm.dll, RVA 0x8306b0,
        // resolved against the PDB; wildcarded registers make it relocatable across jvm.dll loads).


        static SymbolTableNewSymbolFn find_symbol_table_new_symbol() {
            HMODULE jvm = GetModuleHandleA("jvm.dll");
            if (!jvm) return nullptr;
            uint8_t *addr = scanner::scan(jvm, kSymbolTableNewSymbolPattern);
            return reinterpret_cast<SymbolTableNewSymbolFn>(addr);
        }

        // Intern the string through the real SymbolTable.  Returns nullptr if
        // the native entry is unavailable (e.g. not loaded yet / pattern miss)
        // or the string is empty (empty Utf8 constants are legitimate but the
        // native SymbolTable entry for them is awkward); the caller then falls
        // back to a best-effort pool scan / fresh allocation.
        static Symbol *intern_symbol(const std::string &utf8) {
            if (utf8.empty()) return nullptr;
            static SymbolTableNewSymbolFn fn = find_symbol_table_new_symbol();
            if (!fn) return nullptr;
            return fn(utf8.data(), static_cast<int>(utf8.size()));
        }

        static Symbol *find_or_alloc_symbol(MergeState &s, const std::string &utf8) {
            auto it = s.utf8_cache.find(utf8);
            if (it != s.utf8_cache.end()) {
                return it->second;
            }

            // Search the old (loaded) constant pool for a Utf8 with equal content.
            ConstantPool *old_cp = s.old_cp;
            if (old_cp && old_cp->tags()) {
                int old_len = old_cp->length();
                for (int i = 1; i < old_len; i++) {
                    if (old_cp->tags()->at(i) == JVM_CONSTANT_Utf8) {
                        Symbol *sym = old_cp->symbol_at(i);
                        if (sym && sym->utf8_length() == static_cast<int>(utf8.size()) &&
                            memcmp(sym->base(), utf8.data(), utf8.size()) == 0) {
                            s.utf8_cache[utf8] = sym;
                            return sym;
                        }
                    }
                }
            }

            // HotSpot resolves classes/fields/methods by interned Symbol identity,
            // so a fresh (non-interned) Symbol would never match.  Intern the string
            // through the real SymbolTable when possible.
            if (Symbol *interned = intern_symbol(utf8)) {
                s.utf8_cache[utf8] = interned;
                return interned;
            }

            // If the native SymbolTable entry is unavailable, fall back to a fresh
            // allocation (won't match by identity, but keeps the merge running).
            Symbol *sym = alloc_symbol(reinterpret_cast<const uint8_t *>(utf8.data()), utf8.size());
            if (sym) {
                s.utf8_cache[utf8] = sym;
            }
            return sym;
        }

        static int find_or_append(MergeState &s, int parsed_index);

        static int append_entry(MergeState &s, int parsed_index) {
            const ParsedCpEntry &e = s.parsed->cp[parsed_index];
            int mi = s.merged_length++;
            int slot_count = 1;

            switch (e.tag) {
                case JVM_CONSTANT_Utf8: {
                    Symbol *sym = find_or_alloc_symbol(s, e.utf8);
                    if (!sym) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Utf8);
                    s.merged_cp->symbol_at_put(mi, sym);
                    break;
                }
                case JVM_CONSTANT_Integer:
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Integer);
                    s.merged_cp->int_at_put(mi, static_cast<jint>(e.int_value));
                    break;
                case JVM_CONSTANT_Float:
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Float);
                    s.merged_cp->int_at_put(mi, static_cast<jint>(e.int_value));
                    break;
                case JVM_CONSTANT_Long:
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Long);
                    s.merged_cp->long_at_put(mi, static_cast<jlong>(e.long_value));
                    slot_count = 2;
                    break;
                case JVM_CONSTANT_Double:
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Double);
                    s.merged_cp->long_at_put(mi, static_cast<jlong>(e.long_value));
                    slot_count = 2;
                    break;
                case JVM_CONSTANT_Class: {
                    int name_idx = find_or_append(s, e.ref1);
                    if (name_idx == 0) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_UnresolvedClass);
                    s.merged_cp->unresolved_klass_at_put(mi, name_idx, s.next_rk_index++);
                    break;
                }
                case JVM_CONSTANT_String: {
                    int utf8_idx = find_or_append(s, e.ref1);
                    if (utf8_idx == 0) return fail_append(s, parsed_index);
                    Symbol *sym = s.merged_cp->symbol_at(utf8_idx);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_String);
                    s.merged_cp->string_at_put(mi, sym);
                    break;
                }
                case JVM_CONSTANT_Fieldref: {
                    int class_idx = find_or_append(s, e.ref1);
                    int nat_idx = find_or_append(s, e.ref2);
                    if (!class_idx || !nat_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Fieldref);
                    s.merged_cp->field_at_put(mi, class_idx, nat_idx);
                    break;
                }
                case JVM_CONSTANT_Methodref: {
                    int class_idx = find_or_append(s, e.ref1);
                    int nat_idx = find_or_append(s, e.ref2);
                    if (!class_idx || !nat_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Methodref);
                    s.merged_cp->method_at_put(mi, class_idx, nat_idx);
                    break;
                }
                case JVM_CONSTANT_InterfaceMethodref: {
                    int class_idx = find_or_append(s, e.ref1);
                    int nat_idx = find_or_append(s, e.ref2);
                    if (!class_idx || !nat_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_InterfaceMethodref);
                    s.merged_cp->interface_method_at_put(mi, class_idx, nat_idx);
                    break;
                }
                case JVM_CONSTANT_NameAndType: {
                    int name_idx = find_or_append(s, e.ref1);
                    int sig_idx = find_or_append(s, e.ref2);
                    if (!name_idx || !sig_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_NameAndType);
                    s.merged_cp->name_and_type_at_put(mi, name_idx, sig_idx);
                    break;
                }
                case JVM_CONSTANT_MethodHandle: {
                    int ref_idx = find_or_append(s, e.ref1);
                    if (!ref_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_MethodHandle);
                    s.merged_cp->method_handle_index_at_put(mi, e.ref_kind, ref_idx);
                    break;
                }
                case JVM_CONSTANT_MethodType: {
                    int desc_idx = find_or_append(s, e.ref1);
                    if (!desc_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, JVM_CONSTANT_MethodType);
                    s.merged_cp->method_type_index_at_put(mi, desc_idx);
                    break;
                }
                case JVM_CONSTANT_Dynamic:
                case JVM_CONSTANT_InvokeDynamic: {
                    int nat_idx = find_or_append(s, e.ref2);
                    if (!nat_idx) return fail_append(s, parsed_index);

                    int bsm_idx = -1;
                    if (e.ref1 < s.bsm_index_map->size()) {
                        bsm_idx = (*s.bsm_index_map)[e.ref1];
                    }
                    if (bsm_idx < 0) {
                        if (e.ref1 >= s.parsed->bootstrap_methods.size()) return fail_append(s, parsed_index);
                        BootstrapMethod mapped = s.parsed->bootstrap_methods[e.ref1];
                        // remap bootstrap method ref and args CP indices to merged pool
                        int merged_bsm_ref = find_or_append(s, static_cast<int>(mapped.bsm_ref));
                        if (!merged_bsm_ref) return fail_append(s, parsed_index);
                        mapped.bsm_ref = static_cast<u2>(merged_bsm_ref);
                        for (u2 &arg: mapped.args) {
                            int merged_arg = find_or_append(s, static_cast<int>(arg));
                            if (!merged_arg) return fail_append(s, parsed_index);
                            arg = static_cast<u2>(merged_arg);
                        }
                        bsm_idx = append_bootstrap_method(s.operands, mapped);
                        if (e.ref1 < s.bsm_index_map->size()) {
                            (*s.bsm_index_map)[e.ref1] = bsm_idx;
                        }
                    }

                    if (e.tag == JVM_CONSTANT_Dynamic) {
                        s.merged_cp->tag_at_put(mi, JVM_CONSTANT_Dynamic);
                        s.merged_cp->dynamic_constant_at_put(mi, bsm_idx, nat_idx);
                    } else {
                        s.merged_cp->tag_at_put(mi, JVM_CONSTANT_InvokeDynamic);
                        s.merged_cp->invoke_dynamic_at_put(mi, bsm_idx, nat_idx);
                    }
                    break;
                }
                case JVM_CONSTANT_Module:
                case JVM_CONSTANT_Package: {
                    int name_idx = find_or_append(s, e.ref1);
                    if (!name_idx) return fail_append(s, parsed_index);
                    s.merged_cp->tag_at_put(mi, e.tag);
                    s.merged_cp->int_at_put(mi, name_idx);
                    break;
                }
                default:
                    return fail_append(s, parsed_index);
            }

            (*s.index_map)[parsed_index] = mi;
            if (slot_count == 2) {
                (*s.index_map)[parsed_index + 1] = mi + 1;
                s.merged_length++;
            }
            return mi;
        }

        static int find_or_append(MergeState &s, int parsed_index) {
            if (parsed_index <= 0 || parsed_index >= static_cast<int>(s.parsed->cp.size())) {
                return 0;
            }
            if (s.index_map->at(parsed_index) >= 0) {
                return s.index_map->at(parsed_index);
            }
            // Entry not yet merged; append it (and its dependencies).
            return append_entry(s, parsed_index);
        }

        static int count_class_entries(const std::vector<ParsedCpEntry> &cp) {
            int n = 0;
            for (size_t i = 1; i < cp.size(); i++) {
                if (cp[i].tag == JVM_CONSTANT_Class) n++;
            }
            return n;
        }

        static bool copy_old_cp_entry(ConstantPool *old_cp, ConstantPool *merged_cp, int old_index, int new_index,
                                      int old_rk_length, int &next_rk) {
            u1 tag = old_cp->tags()->at(old_index);
            merged_cp->tag_at_put(new_index, tag);

            switch (tag) {
                case JVM_CONSTANT_Utf8:
                case JVM_CONSTANT_String:
                    merged_cp->symbol_at_put(new_index, old_cp->symbol_at(old_index));
                    break;
                case JVM_CONSTANT_Integer:
                case JVM_CONSTANT_Float:
                case JVM_CONSTANT_Fieldref:
                case JVM_CONSTANT_Methodref:
                case JVM_CONSTANT_InterfaceMethodref:
                case JVM_CONSTANT_NameAndType:
                case JVM_CONSTANT_MethodHandle:
                case JVM_CONSTANT_MethodType:
                case JVM_CONSTANT_Dynamic:
                case JVM_CONSTANT_InvokeDynamic:
                case JVM_CONSTANT_ClassIndex:
                case JVM_CONSTANT_StringIndex:
                case JVM_CONSTANT_Module:
                case JVM_CONSTANT_Package:
                    merged_cp->int_at_put(new_index, *old_cp->int_at_addr(old_index));
                    break;
                case JVM_CONSTANT_Long:
                case JVM_CONSTANT_Double:
                    merged_cp->long_at_put(new_index, *old_cp->long_at_addr(old_index));
                    break;
                case JVM_CONSTANT_Class:
                case JVM_CONSTANT_UnresolvedClass:
                case JVM_CONSTANT_UnresolvedClassInError: {
                    int raw = *old_cp->int_at_addr(old_index);
                    int name_idx = (raw >> 16) & 0xffff;
                    int rk_idx = raw & 0xffff;
                    // keep the existing resolved klass slot
                    merged_cp->unresolved_klass_at_put(new_index, name_idx, rk_idx);
                    break;
                }
                default:
                    // Copy raw word for unknown tags.
                    merged_cp->obj_at_addr(new_index)[0] = old_cp->obj_at_addr(old_index)[0];
                    break;
            }
            return true;
        }

        // ----------------------------------------------------------------------------
        // Bytecode helpers
        // ----------------------------------------------------------------------------

        static int bytecode_operand_size(uint8_t op) {
            switch (op) {
                case 0x12:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x84:
                case 0xA9:
                case 0xBC:
                    return 1;
                case 0x11:
                case 0x13:
                case 0x14:
                case 0xA7:
                case 0xA8:
                case 0xB2:
                case 0xB3:
                case 0xB4:
                case 0xB5:
                case 0xB6:
                case 0xB7:
                case 0xB8:
                case 0xBB:
                case 0xBD:
                case 0xC0:
                case 0xC1:
                case 0xC6:
                case 0xC7:
                    return 2;
                case 0xB9:
                case 0xBA:
                case 0xC8:
                case 0xC9:
                    return 4;
                case 0xC5:
                    return 3;
                default:
                    return 0;
            }
        }

        // Signed reads/writes (big-endian) for branch / switch operands.
        inline int32_t read_s4_be(const uint8_t *p) {
            return static_cast<int32_t>((static_cast<uint32_t>(p[0]) << 24) |
                                        (static_cast<uint32_t>(p[1]) << 16) |
                                        (static_cast<uint32_t>(p[2]) << 8) |
                                        static_cast<uint32_t>(p[3]));
        }

        inline int16_t read_s2_be(const uint8_t *p) {
            return static_cast<int16_t>(static_cast<uint16_t>((p[0] << 8) | p[1]));
        }

        inline void write_s4_be(uint8_t *p, int32_t v) {
            uint32_t u = static_cast<uint32_t>(v);
            p[0] = static_cast<uint8_t>(u >> 24);
            p[1] = static_cast<uint8_t>(u >> 16);
            p[2] = static_cast<uint8_t>(u >> 8);
            p[3] = static_cast<uint8_t>(u);
        }

        inline void write_s2_be(uint8_t *p, int16_t v) {
            uint16_t u = static_cast<uint16_t>(v);
            p[0] = static_cast<uint8_t>(u >> 8);
            p[1] = static_cast<uint8_t>(u);
        }

        // Number of bytes the switch keeps aligned to a 4-byte boundary relative
        // to the start of the Code array (JVMS): the first int operand must sit on
        // a 4-byte boundary from the method start.
        inline size_t switch_padding(size_t opcode_offset) {
            return (4 - ((opcode_offset + 1) % 4)) % 4;
        }

        // Total length in bytes of the instruction starting at bc[off].  Handles
        // the variable-length instructions whose size bytecode_operand_size()
        // cannot express (tableswitch/lookupswitch alignment padding, wide, and
        // the vm's internal _fast_aldc forms).
        static size_t instruction_length(const std::vector<uint8_t> &bc, size_t off) {
            uint8_t op = bc[off];
            switch (op) {
                case 0xAA: // tableswitch
                case 0xAB: {
                    // lookupswitch
                    if (off + 1 >= bc.size()) return bc.size() - off;
                    size_t pad = switch_padding(off);
                    size_t base = off + 1 + pad;
                    if (base + 8 > bc.size()) return bc.size() - off;
                    if (op == 0xAA) {
                        // default, low, high then (high - low + 1) int offsets
                        int32_t lo = read_s4_be(&bc[base + 4]);
                        int32_t hi = read_s4_be(&bc[base + 8]);
                        if (hi < lo) return bc.size() - off;
                        uint64_t npairs = static_cast<uint64_t>(hi) - lo + 1;
                        size_t end = base + 12 + npairs * 4;
                        return (end <= bc.size()) ? end - off : bc.size() - off;
                    } else {
                        // default, npairs then npairs (match, offset) int pairs
                        int32_t npairs = read_s4_be(&bc[base + 4]);
                        if (npairs < 0) return bc.size() - off;
                        size_t end = base + 8 + static_cast<size_t>(npairs) * 8;
                        return (end <= bc.size()) ? end - off : bc.size() - off;
                    }
                }
                case 0xC4: {
                    // wide
                    if (off + 1 >= bc.size()) return bc.size() - off;
                    uint8_t wide_op = bc[off + 1];
                    return (wide_op == 0x84) ? 6 : 4; // wide iinc has extra operand
                }
                case 0xE6: // _fast_aldc (internal): opcode + u1 index
                    return 2;
                case 0xE7: // _fast_aldc_w (internal): opcode + u2 index
                    return 3;
                default:
                    return 1 + bytecode_operand_size(op);
            }
        }

        // Rewrite a single instruction's constant-pool operand(s) in place,
        // given its original opcode (head already emitted at out+i when emitted).
        static void rewrite_operand(uint8_t *out, size_t i, uint8_t op, const std::vector<int> &index_map) {
            auto map_idx = [&](int idx) {
                return (idx >= 0 && idx < static_cast<int>(index_map.size())) ? index_map[idx] : idx;
            };
            if (op == 0x12) {
                out[i + 1] = static_cast<uint8_t>(map_idx(out[i + 1]));
            } else if (op == 0x13 || op == 0x14) {
                write_u2_be(&out[i + 1], static_cast<u2>(map_idx((out[i + 1] << 8) | out[i + 2])));
            } else if ((op >= 0xB2 && op <= 0xB8) || op == 0xBB || op == 0xBD || op == 0xC0 || op == 0xC1 || op ==
                       0xC5) {
                write_u2_be(&out[i + 1], static_cast<u2>(map_idx((out[i + 1] << 8) | out[i + 2])));
            } else if (op == 0xB9 || op == 0xBA) {
                write_u2_be(&out[i + 1], static_cast<u2>(map_idx((out[i + 1] << 8) | out[i + 2])));
            }
        }

        static bool is_branch(uint8_t op) {
            return (op >= 0x99 && op <= 0xA8) || op == 0xC6 || op == 0xC7 || op == 0xC8 || op == 0xC9;
        }

        // TODO(user): replace with the correct byte pattern for Rewriter::rewrite in your JDK build.

        // stub pattern

        using RewriterRewriteFn = void (*)(InstanceKlass *klass, void *thread);

        RewriterRewriteFn find_rewriter_rewrite() {
            HMODULE jvm = GetModuleHandleA("jvm.dll");
            if (!jvm) return nullptr;
            uint8_t *addr = scanner::scan(jvm, kRewriterPattern);
            return reinterpret_cast<RewriterRewriteFn>(addr);
        }
    } // anonymous namespace

    // ----------------------------------------------------------------------------
    // Public API
    // ----------------------------------------------------------------------------

    RedefineResult merge_constant_pool(ConstantPool *old_cp,
                                       const std::vector<uint8_t> &new_class_bytes) {
        RedefineResult result;
        result.success = false;

        if (!old_cp) {
            result.error = "old_cp is null";
            return result;
        }
        assert(!new_class_bytes.empty(), "new class bytes are empty");
        assert(old_cp->length() > 0, "old constant pool has zero length");

        ParsedClassFile parsed;
        std::string parse_error;
            ClassFileReader reader(new_class_bytes);
        if (!reader.parse(parsed, parse_error)) {
            result.error = "class parse error: " + parse_error;
            return result;
        }
        assert(parsed.cp.size() > 1, "parsed class has no constant pool entries");

        int old_len = old_cp->length();
        int parsed_len = static_cast<int>(parsed.cp.size());

        // Conservative upper bound.
        int max_len = old_len + parsed_len * 2;

        ConstantPool *merged = alloc_constant_pool(old_cp, max_len);
        if (!merged) {
            result.error = "failed to allocate merged constant pool";
            return result;
        }

        // Allocate new tags array.
        Array<u1> *new_tags = alloc_array<u1>(max_len);
        if (!new_tags) {
            result.error = "failed to allocate tags array";
            return result;
        }
        assert(new_tags->length() >= max_len, "tags array smaller than requested");
        if (old_cp->tags()) {
            int old_tags_len = old_cp->tags()->length();
            assert(old_tags_len <= max_len, "old tags array larger than merged pool");
            for (int i = 0; i < old_tags_len && i < max_len; i++) {
                new_tags->at_put(i, old_cp->tags()->at(i));
            }
        }
        merged->set_tags(new_tags);

        // Allocate resolved_klasses array.
        int old_rk_len = old_cp->resolved_klasses() ? old_cp->resolved_klasses()->length() : 0;
        int new_class_count = count_class_entries(parsed.cp);
        int rk_len = old_rk_len + new_class_count;
        assert(rk_len > 0, "resolved_klasses length is zero");
        Array<InstanceKlass *> *new_rk = alloc_array<InstanceKlass *>(rk_len + 4);
        if (!new_rk) {
            result.error = "failed to allocate resolved_klasses array";
            return result;
        }
        assert(new_rk->length() >= rk_len, "resolved_klasses array smaller than requested");
        if (old_cp->resolved_klasses()) {
            for (int i = 0; i < old_rk_len; i++) {
                new_rk->at_put(i, old_cp->resolved_klasses()->at(i));
            }
        }
        merged->set_resolved_klasses(new_rk);

        // Copy old entries into merged at same indices.
        int next_rk = old_rk_len;
        for (int i = 1; i < old_len; i++) {
            copy_old_cp_entry(old_cp, merged, i, i, old_rk_len, next_rk);
            u1 tag = old_cp->tags()->at(i);
            if (tag == JVM_CONSTANT_Long || tag == JVM_CONSTANT_Double) {
                i++;
            }
        }

        // Prepare operands builder from old operands.
        OperandsBuilder operands_builder;
        copy_old_operands(old_cp->operands(), operands_builder);

        // Build index map for parsed entries.
        std::vector<int> index_map(parsed_len, -1);
        std::vector<int> bsm_index_map(parsed.bootstrap_methods.size(), -1);

        MergeState state;
        state.old_cp = old_cp;
        state.merged_cp = merged;
        state.parsed = &parsed;
        state.index_map = &index_map;
        state.bsm_index_map = &bsm_index_map;
        state.merged_length = old_len;
        state.next_rk_index = next_rk;
        state.operands = operands_builder;

        // Append every parsed entry that is not already present.
        for (int i = 1; i < parsed_len; i++) {
            if ((*state.index_map)[i] >= 0) continue; // trailing slot already mapped
            int appended = find_or_append(state, i);
            if (appended <= 0) {
                if (state.error.empty()) {
                    state.error = "failed to append parsed constant-pool index " + std::to_string(i);
                }
                result.error = state.error;
                return result;
            }
        }

        // Install operands array.
        Array<u2> *new_operands = build_operands_array(state.operands);
        if (new_operands) {
            merged->set_operands(new_operands);
        } else if (old_cp->operands() == nullptr) {
            merged->set_operands(nullptr);
        }

        assert(state.merged_length <= max_len, "merged pool grew past its upper bound");
        // Trim length to actual used entries.
        merged->set_length(state.merged_length);
        new_tags->set_length(state.merged_length);
        // Keep arrays at allocated size (larger than length); that is fine.

        // Reset cache so the native Rewriter can rebuild it.
        merged->set_cache(nullptr);

        result.success = true;
        result.new_cp = merged;
        result.index_map = std::move(index_map);
        return result;
    }

    // Remap one exception table in place:
    //  - catch_type is a raw CP index into the new class -> merged pool index.
    //  - start_pc/end_pc/handler_pc are offsets in the old bytecode; when the
    //    stream is rebuilt (ldc widening) they must follow the relocation map.
    //  - end_pc may equal the old code length; the sentinel maps it to the new one.
    static void remap_exception_table(std::vector<ExceptionTableElement> &table,
                                      const std::vector<int> &index_map,
                                      const std::unordered_map<size_t, size_t> *off_map) {
        if (table.empty()) return;
        auto map_off = [&](u2 pc) -> u2 {
            if (off_map == nullptr) return pc;
            auto it = off_map->find(static_cast<size_t>(pc));
            if (it != off_map->end()) return static_cast<u2>(it->second);
            return pc;
        };
        for (auto &e: table) {
            e.start_pc = map_off(e.start_pc);
            e.end_pc = map_off(e.end_pc);
            e.handler_pc = map_off(e.handler_pc);
            if (e.catch_type_index != 0 &&
                e.catch_type_index < static_cast<u2>(index_map.size())) {
                int merged = index_map[e.catch_type_index];
                e.catch_type_index = static_cast<u2>(merged >= 0 ? merged : 0);
            }
        }
    }

    bool rewrite_bytecode_cp_indices(std::vector<uint8_t> &bytecode,
                                     const std::vector<int> &index_map,
                                     std::vector<ExceptionTableElement> *exception_table) {
        // Decode every instruction once, recording its opcode, offset in the old
        // stream, length (including switch/wide handling) and whether an ldc needs
        // widening to ldc_w.  Branch / switch targets are resolved in a second pass
        // against this table, so they survive any ldc widening.
        struct Instr {
            uint8_t op;
            size_t old_off = 0;
            size_t old_len = 0;
            size_t new_off = 0;
            size_t new_len = 0;
            bool widen = false;
            bool is_switch = false;
            size_t sw_base = 0; // offset of first int operand in the OLD stream
            size_t sw_pad = 0; // old alignment padding
        };
        std::vector<Instr> instrs;

        {
            size_t off = 0;
            const size_t size = bytecode.size();
            while (off < size) {
                Instr in;
                in.op = bytecode[off];
                in.old_off = off;
                in.old_len = instruction_length(bytecode, off);
                if (in.op == 0x12) {
                    int merged = (bytecode[off + 1] >= 0 && bytecode[off + 1] < static_cast<int>(index_map.size()))
                                     ? index_map[bytecode[off + 1]]
                                     : bytecode[off + 1];
                    if (merged > 0xFF) in.widen = true;
                } else if (in.op == 0xAA || in.op == 0xAB) {
                    in.is_switch = true;
                    in.sw_pad = switch_padding(off);
                    in.sw_base = off + 1 + in.sw_pad;
                }
                instrs.push_back(in);
                off += in.old_len;
            }
        }

        // Validate that every constant-pool operand maps to a merged index. A
        // hole (index_map == -1) would otherwise be emitted as 0xFFFF and crash
        // the native Rewriter on a tags[] read far past the end of its allocation
        // (EXCEPTION_ACCESS_VIOLATION in add_invokedynamic_resolved_references_entry).
        for (const Instr &in: instrs) {
            if (in.is_switch) continue;
            int idx;
            if (in.op == 0x12) {
                idx = bytecode[in.old_off + 1];
            } else if (in.op == 0x13 || in.op == 0x14 ||
                       (in.op >= 0xB2 && in.op <= 0xB8) || in.op == 0xBB ||
                       in.op == 0xBD || in.op == 0xC0 || in.op == 0xC1 ||
                       in.op == 0xC5 || in.op == 0xB9 || in.op == 0xBA) {
                idx = (bytecode[in.old_off + 1] << 8) | bytecode[in.old_off + 2];
            } else {
                continue;
            }
            int mapped = (idx >= 1 && idx < static_cast<int>(index_map.size())) ? index_map[idx] : -1;
            if (mapped < 0) {
                std::cerr << "redefine: constant-pool index " << idx << " (opcode 0x" << std::hex
                          << static_cast<int>(in.op) << std::dec << ") has no merged-pool mapping; "
                          << "index_map.size()=" << index_map.size()
                          << (idx >= static_cast<int>(index_map.size())
                                  ? " (operand out of range for this class)"
                                  : " (operand in range but unmapped)")
                          << "; aborting" << std::endl;
                return false;
            }
        }

        bool needs_widen = false;
        for (const Instr &in: instrs) {
            if (in.widen) {
                needs_widen = true;
                break;
            }
        }

        if (!needs_widen) {
            // No instruction grows: rewrite operands in place.  Offsets unchanged.
            for (const Instr &in: instrs) {
                if (in.is_switch) continue; // no CP operands
                rewrite_operand(bytecode.data(), in.old_off, in.op, index_map);
            }
            if (exception_table) remap_exception_table(*exception_table, index_map, nullptr);
            return true;
        }

        // -------- Widening pass: rebuild the stream, relocating branch/switch targets.
        // Compute new offsets and lengths first.
        size_t new_size = 0;
        for (Instr &in: instrs) {
            in.new_off = new_size;
            if (in.widen) {
                // ldc(2) -> ldc_w(3)
                in.new_len = in.old_len + 1;
            } else if (in.is_switch) {
                // Re-align padding to the new offset; data (operand words) unchanged.
                size_t data_len = in.old_len - 1 - in.sw_pad;
                size_t pad = switch_padding(in.new_off);
                in.new_len = 1 + pad + data_len;
            } else {
                in.new_len = in.old_len;
            }
            new_size += in.new_len;
        }

        // Old-offset -> new-offset map for branch targets (instruction starts).
        std::unordered_map<size_t, size_t> off_map;
        for (const Instr &in: instrs) {
            off_map.emplace(in.old_off, in.new_off);
        }
        // Exception tables may reference the exclusive end of the code range.
        off_map.emplace(bytecode.size(), new_size);

        std::vector<uint8_t> out(new_size, 0);

        for (const Instr &in: instrs) {
            const size_t src = in.old_off;
            const size_t dst = in.new_off;

            if (in.widen) {
                // ldc -> ldc_w
                int merged = (bytecode[src + 1] >= 0 && bytecode[src + 1] < static_cast<int>(index_map.size()))
                                 ? index_map[bytecode[src + 1]]
                                 : bytecode[src + 1];
                out[dst] = 0x13;
                write_u2_be(&out[dst + 1], static_cast<u2>(merged));
                continue;
            }

            if (in.is_switch) {
                // Copy full old instruction, then fix the 4-byte jump offsets using
                // old/new alignment.
                memcpy(&out[dst], &bytecode[src], in.old_len);
                size_t pad = switch_padding(dst);
                size_t new_base = dst + 1 + pad;

                // default offset (s4) at base
                int32_t def = read_s4_be(&out[new_base]);
                write_s4_be(&out[new_base],
                            static_cast<int32_t>(new_base + in.new_off - in.old_off)); // placeholder; fixed below
                (void) def;

                if (in.op == 0xAA) {
                    int32_t lo = read_s4_be(&out[new_base + 4]);
                    int32_t hi = read_s4_be(&out[new_base + 8]);
                    uint64_t n = static_cast<uint64_t>(hi) - lo + 1;
                    for (uint64_t k = 0; k < n; k++) {
                        size_t pos = new_base + 12 + k * 4;
                        // copy from old position
                        size_t old_pos = in.sw_base + 12 + k * 4;
                        write_s4_be(&out[pos], read_s4_be(&bytecode[old_pos]));
                    }
                } else {
                    int32_t npairs = read_s4_be(&out[new_base + 4]);
                    for (int32_t k = 0; k < npairs; k++) {
                        size_t pos = new_base + 8 + static_cast<size_t>(k) * 8;
                        size_t old_pos = in.sw_base + 8 + static_cast<size_t>(k) * 8;
                        write_s4_be(&out[pos], read_s4_be(&bytecode[old_pos]));
                        write_s4_be(&out[pos + 4], read_s4_be(&bytecode[old_pos + 4]));
                    }
                }
                continue;
            }

            // Regular instruction: copy whole thing, then rewrite CP operand.
            memcpy(&out[dst], &bytecode[src], in.old_len);
            rewrite_operand(out.data(), dst, in.op, index_map);
        }

        // Relocate branch / switch targets.
        for (const Instr &in: instrs) {
            const size_t dst = in.new_off;
            if (in.is_switch) {
                size_t pad = switch_padding(dst);
                size_t base = dst + 1 + pad;
                auto reloc = [&](uint8_t *p) {
                    int32_t tgt = read_s4_be(p) + static_cast<int32_t>(in.old_off);
                    auto it = off_map.find(static_cast<size_t>(tgt));
                    if (it != off_map.end()) {
                        write_s4_be(p, static_cast<int32_t>(it->second) - static_cast<int32_t>(dst));
                    }
                };
                reloc(&out[base]); // default
                if (in.op == 0xAA) {
                    int32_t lo = read_s4_be(&out[base + 4]);
                    int32_t hi = read_s4_be(&out[base + 8]);
                    uint64_t n = static_cast<uint64_t>(hi) - lo + 1;
                    for (uint64_t k = 0; k < n; k++) {
                        reloc(&out[base + 12 + k * 4]);
                    }
                } else {
                    int32_t npairs = read_s4_be(&out[base + 4]);
                    for (int32_t k = 0; k < npairs; k++) {
                        reloc(&out[base + 8 + static_cast<size_t>(k) * 8 + 4]);
                    }
                }
            } else if (is_branch(in.op)) {
                int off_len = (in.op == 0xC8 || in.op == 0xC9) ? 4 : 2;
                uint8_t *p = &out[dst + 1];
                int32_t rel = (off_len == 4) ? read_s4_be(p) : read_s2_be(p);
                int32_t tgt = rel + static_cast<int32_t>(in.old_off);
                auto it = off_map.find(static_cast<size_t>(tgt));
                if (it != off_map.end()) {
                    int32_t new_rel = static_cast<int32_t>(it->second) - static_cast<int32_t>(dst);
                    if (off_len == 4) {
                        write_s4_be(p, new_rel);
                    } else {
                        write_s2_be(p, static_cast<int16_t>(new_rel));
                    }
                }
            }
        }

        bytecode = std::move(out);
        if (exception_table) remap_exception_table(*exception_table, index_map, &off_map);
        return true;
    }

    bool restore_method_bytecodes(Method *method, ConstantPool *cp) {
        if (!method || !cp) return false;
        ConstMethod *cm = method->constMethod();
        if (!cm) return false;
        uint8_t *code = static_cast<uint8_t *>(cm->code_base());
        int code_size = cm->code_size();
        if (!code || code_size <= 0) return false;

        ConstantPoolCache *cache = cp->cache();
        Array<u2> *ref_map = cache ? cache->reference_map() : nullptr;

        int i = 0;
        while (i < code_size) {
            uint8_t op = code[i];
            int op_len = 1 + bytecode_operand_size(op);

            if (op == 0xE6) {
                // _fast_aldc
                if (!ref_map) return false;
                int ref_idx = code[i + 1];
                int cp_idx = (ref_idx >= 0 && ref_idx < ref_map->length()) ? ref_map->at(ref_idx) : -1;
                if (cp_idx < 0 || cp_idx > 0xFF) return false;
                code[i] = 0x12; // ldc
                code[i + 1] = static_cast<uint8_t>(cp_idx);
            } else if (op == 0xE7) {
                // _fast_aldc_w
                if (!ref_map) return false;
                int ref_idx = read_u2_native(&code[i + 1]);
                int cp_idx = (ref_idx >= 0 && ref_idx < ref_map->length()) ? ref_map->at(ref_idx) : -1;
                if (cp_idx < 0) return false;
                code[i] = 0x13; // ldc_w
                write_u2_be(&code[i + 1], static_cast<u2>(cp_idx));
            } else if (op >= 0xB2 && op <= 0xB8) {
                // get/put/invoke
                if (!cache) return false;
                int cache_idx = read_u2_native(&code[i + 1]);
                if (cache_idx < 0 || cache_idx >= cache->length()) return false;
                ConstantPoolCacheEntry *entry = cache->entry_at(cache_idx);
                int cp_idx = entry ? entry->constant_pool_index() : -1;
                if (cp_idx < 0) return false;
                write_u2_be(&code[i + 1], static_cast<u2>(cp_idx));
            } else if (op == 0xB9) {
                // invokeinterface
                if (!cache) return false;
                int cache_idx = read_u2_native(&code[i + 1]);
                if (cache_idx < 0 || cache_idx >= cache->length()) return false;
                ConstantPoolCacheEntry *entry = cache->entry_at(cache_idx);
                int cp_idx = entry ? entry->constant_pool_index() : -1;
                if (cp_idx < 0) return false;
                write_u2_be(&code[i + 1], static_cast<u2>(cp_idx));
                // leave count and zero bytes untouched
            } else if (op == 0xBA) {
                // invokedynamic
                if (!cache) return false;
                u4 encoded = read_u4_native(&code[i + 1]);
                int indy_idx = ~static_cast<int>(encoded);
                ResolvedIndyEntry *indy = cache->resolved_indy_entry_at(indy_idx);
                if (!indy) return false;
                int cp_idx = indy->constant_pool_index();
                if (cp_idx < 0) return false;
                code[i] = 0xBA; // invokedynamic
                write_u2_be(&code[i + 1], static_cast<u2>(cp_idx));
                code[i + 3] = 0;
                code[i + 4] = 0;
            } else if (op == static_cast<uint8_t>(Bytecodes::_invokehandle)) {
                if (!cache) return false;
                int cache_idx = read_u2_native(&code[i + 1]);
                if (cache_idx < 0 || cache_idx >= cache->length()) return false;
                ConstantPoolCacheEntry *entry = cache->entry_at(cache_idx);
                int cp_idx = entry ? entry->constant_pool_index() : -1;
                if (cp_idx < 0) return false;
                code[i] = 0xB6; // restore as invokevirtual (sig-poly final method)
                write_u2_be(&code[i + 1], static_cast<u2>(cp_idx));
            }

            i += op_len;
        }
        return true;
    }

    bool invoke_native_rewriter(InstanceKlass *ik) {
        assert(ik != nullptr, "instance klass is null");
        RewriterRewriteFn fn = find_rewriter_rewrite();
        if (!fn) {
            return false;
        }
        // The native Rewriter::rewrite(InstanceKlass*, Thread*) needs a real,
        // current JavaThread* (it uses ResourceMark / constantPoolHandle /
        // CHECK on THREAD).  Resolve it the same way HotSpot's Thread::current()
        // does from the embedded JNIEnv.
        void *thread = JavaThread::CurrentThread();
        if (thread == nullptr) {
            return false;
        }
        fn(ik, thread);
        ConstantPool *cp = ik->constants();
        assert(cp != nullptr, "class has null constant pool after rewrite");
        ConstantPoolCache *cache = cp->cache();
        assert(cache != nullptr, "rewriter did not build a constant pool cache");
        cache->set_constant_pool(cp);
        return true;
    }

    bool redefine_method(InstanceKlass *ik,
                         const char *method_name,
                         const char *method_signature,
                         const std::vector<uint8_t> &new_class_bytes) {
        assert(ik != nullptr, "instance klass is null");
        assert(method_name != nullptr, "method name is null");
        assert(method_signature != nullptr, "method signature is null");
        assert(!new_class_bytes.empty(), "new class bytes are empty");
        ConstantPool *old_cp = ik->constants();
        if (!old_cp) return false;

        // Park every Java thread so none is mid-execution of the method we are
        // about to rewrite (RestoreMethodPoolSwap-style mutation). Degrades to a
        // no-op if the internal safepoint cannot be taken. The long VirtualAlloc
        // merge below still runs inside the guard window: it is short.
        SafepointGuard guard;

        // 1. Reverse existing methods so the native Rewriter sees raw CP indices.
        Array<Method *> *methods = ik->methods();
        if (methods) {
            for (int i = 0; i < methods->length(); i++) {
                if (!restore_method_bytecodes(methods->at(i), old_cp)) {
                    // Non-fatal: some methods may not have cacheable bytecodes.
                }
            }
        }

        // 2. Merge constant pools.
        RedefineResult merge = merge_constant_pool(old_cp, new_class_bytes);
        if (!merge.success) {
            std::cerr << "redefine_method: constant-pool merge failed: " << merge.error << std::endl;
            return false;
        }
        assert(merge.new_cp != nullptr, "merge succeeded but new constant pool is null");

        // 3. Extract target method bytecode from new class bytes.
        ParsedClassFile parsed;
        std::string parse_error;
        ClassFileReader reader(new_class_bytes);
        if (!reader.parse(parsed, parse_error)) {
            std::cerr << "redefine_method: class parse failed: " << parse_error << std::endl;
            return false;
        }

        std::string key(method_name);
        key.push_back('\0');
        key += method_signature;

        std::vector<uint8_t> new_bytecode;
        std::vector<ExceptionTableElement> new_exc_table;
        u2 new_max_stack = 0;
        u2 new_max_locals = 0;
        bool found = false;
        for (const auto &kv: parsed.methods) {
            if (kv.first == key) {
                new_bytecode = kv.second.bytecode;
                new_exc_table = kv.second.exception_table;
                new_max_stack = kv.second.max_stack;
                new_max_locals = kv.second.max_locals;
                found = true;
                break;
            }
        }
        if (!found) return false;

        // 4. Rewrite raw CP indices to merged CP indices.
        if (!rewrite_bytecode_cp_indices(new_bytecode, merge.index_map, &new_exc_table)) return false;

        // 5. Install merged constant pool.
        //    Update the InstanceKlass to point at the new pool.
        {
            static offset cp_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_constants")->
                    offset;
            *reinterpret_cast<ConstantPool **>(reinterpret_cast<uintptr_t>(ik) + cp_offset) = merge.new_cp;
        }

        // 6. Replace target method bytecode.
        Method *target = JvmFunctions::findMethod(ik, method_name, method_signature);
        if (!target) return false;
        target->redefine(std::move(new_bytecode), false, std::move(new_exc_table));
        target->constMethod()->set_max_stack(new_max_stack);
        target->constMethod()->set_max_locals(new_max_locals);

        // 7. Rebuild ConstantPoolCache via native Rewriter::rewrite.
        return invoke_native_rewriter(ik);
    }

    bool redefine_class(InstanceKlass *ik, const std::vector<uint8_t> &new_class_bytes) {
        assert(ik != nullptr, "instance klass is null");
        assert(!new_class_bytes.empty(), "new class bytes are empty");
        ConstantPool *old_cp = ik->constants();
        if (!old_cp) return false;

        // Park every Java thread for the whole mutation (restore, pool merge,
        // bytecode swap, cache rebuild). No-op -> unsynchronized when the
        // internal safepoint cannot be acquired.
        SafepointGuard guard;

        // 1. Merge constant pools once (pure: touches nothing on the class).
        RedefineResult merge = merge_constant_pool(old_cp, new_class_bytes);
        if (!merge.success) {
            std::cerr << "redefine_class: constant-pool merge failed: " << merge.error << std::endl;
            return false;
        }
        assert(merge.new_cp != nullptr, "merge succeeded but new constant pool is null");

        // 2. Parse the new class bytes once to get CP-indexed bytecode per method.
        ParsedClassFile parsed;
        std::string parse_error;
        ClassFileReader reader(new_class_bytes);
        if (!reader.parse(parsed, parse_error)) {
            std::cerr << "redefine_class: class parse failed: " << parse_error << std::endl;
            return false;
        }

        // 3. Rewrite and validate every method up front, BEFORE mutating the
        //    class. A missing mapping aborts here with the class untouched, so
        //    a failure cannot leave the InstanceKlass half-migrated.
        struct PlannedMethod {
            Method *target;
            std::vector<uint8_t> bytecode;
            std::vector<ExceptionTableElement> exc_table;
            u2 max_stack;
            u2 max_locals;
        };
        std::vector<PlannedMethod> plan;
        for (const auto &kv: parsed.methods) {
            std::size_t sep = kv.first.find('\0');
            if (sep == std::string::npos) continue;
            std::string name = kv.first.substr(0, sep);
            std::string sig = kv.first.substr(sep + 1);

            Method *target = JvmFunctions::findMethod(ik, name.c_str(), sig.c_str());
            if (!target) continue; // not present in the old class; skip
            std::vector<uint8_t> new_bytecode = kv.second.bytecode;
            std::vector<ExceptionTableElement> new_exc_table = kv.second.exception_table;
            if (!rewrite_bytecode_cp_indices(new_bytecode, merge.index_map, &new_exc_table)) {
                std::cerr << "redefine_class: method " << name << sig << " failed to rewrite" << std::endl;
                return false;
            }
            plan.push_back({target, std::move(new_bytecode), std::move(new_exc_table),
                            kv.second.max_stack, kv.second.max_locals});
        }
        if (plan.empty()) {
            std::cerr << "redefine_class: no methods from the new class match the old class" << std::endl;
            return false;
        }

        // 4. Reverse existing methods so the native Rewriter sees raw CP indices.
        Array<Method *> *methods = ik->methods();
        if (methods) {
            for (int i = 0; i < methods->length(); i++) {
                restore_method_bytecodes(methods->at(i), old_cp);
            }
        }

        // 5. Install merged constant pool on the class before touching the cache.
        {
            static offset cp_offset = OffsetExtractor::getInstance()->findVMStructEntry("InstanceKlass", "_constants")->
                    offset;
            *reinterpret_cast<ConstantPool **>(reinterpret_cast<uintptr_t>(ik) + cp_offset) = merge.new_cp;
        }

        // 6. Replace bytecode of every method in the plan using merged-CP-rewritten bytecode.
        for (PlannedMethod &p: plan) {
            p.target->constMethod()->set_constant_pool(merge.new_cp);
            p.target->redefine(std::move(p.bytecode), false, std::move(p.exc_table));
            p.target->constMethod()->set_max_stack(p.max_stack);
            p.target->constMethod()->set_max_locals(p.max_locals);
        }

        // 7. Rebuild ConstantPoolCache via native Rewriter::rewrite.
        return invoke_native_rewriter(ik);
    }
} // namespace redefine
