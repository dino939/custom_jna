#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include "../jvm/struct/oop/array/Array.h"
#include "../jvm/struct/oop/constantPool/ConstantPool.h"
#include "../jvm/struct/oop/constantPool/ConstantPoolCache.h"
#include "../jvm/offset/OffsetExtractor.h"
#include "../jvm/reconstruct/bytecodeTables.h"

namespace BytecodeRewriter {
    constexpr size_t CPCacheEntrySize = 4 * sizeof(intptr_t);

    // JVM internal bytecodes used by the interpreter after class linking
    // (see HotSpot bytecodes.hpp: _fast_aldc = 0xE6, _fast_aldc_w = 0xE7).
    constexpr uint8_t FastAldc = 0xE6;
    constexpr uint8_t FastAldcW = 0xE7;

    // Map a JVM-internal (linked) opcode onto its class-file opcode via the
    // _java_code table dumped from jvm.dll (identity for 0x00..0xCA).
    static uint8_t to_java_opcode(uint8_t op) {
        return static_cast<uint8_t>(kJvmJavaCode[op]);
    }

    // Total instruction length (opcode + operands) of a class-file opcode, from the
    // _lengths table low nibble. Returns 0 for variable-length opcodes
    // (tableswitch/lookupswitch -- and wide, resolved via the high nibble).
    static uint8_t instr_length(uint8_t op) {
        return static_cast<uint8_t>(kJvmLengths[op] & 0xF);
    }

    static uintptr_t entries_base(ConstantPoolCache *cache) {
        static uintptr_t cacheSize = 0;
        if (cacheSize == 0) {
            cacheSize = OffsetExtractor::getInstance()->findVMTypeEntry("ConstantPoolCache")->size;
        }
        return reinterpret_cast<uintptr_t>(cache) + cacheSize;
    }

    static int find_cache_index(ConstantPoolCache *cache, int cp_index) {
        const int len = cache->length();
        const uintptr_t entries = entries_base(cache);
        for (int i = 0; i < len; i++) {
            const intptr_t indices = *reinterpret_cast<const intptr_t *>(
                entries + static_cast<size_t>(i) * CPCacheEntrySize);
            const int entry_cp_index = static_cast<int>(static_cast<uint64_t>(indices) & 0xFFFF);
            if (entry_cp_index == cp_index) {
                return i;
            }
        }
        return -1;
    }

    static std::vector<uint8_t> rewrite(InstanceKlass *ik, const std::vector<uint8_t> &payload) {
        ConstantPool *cp = ik->constants();
        ConstantPoolCache *cache = cp->cache();
        std::vector<uint8_t> out = payload;

        size_t i = 0;
        while (i < out.size()) {
            const int op = out[i];

            if (op == 0xaa || op == 0xab) {
                // tableswitch / lookupswitch
                size_t pad = (4 - ((i + 1) % 4)) % 4;
                if (op == 0xaa) {
                    size_t base = i + 1 + pad;
                    int32_t lo = (out[base + 4] << 24) | (out[base + 5] << 16) | (out[base + 6] << 8) | out[base + 7];
                    int32_t hi = (out[base + 8] << 24) | (out[base + 9] << 16) | (out[base + 10] << 8) | out[
                                     base + 11];
                    i += 1 + pad + 4 + 4 + 4 + static_cast<size_t>(hi - lo + 1) * 4;
                } else {
                    size_t base = i + 1 + pad;
                    int32_t npairs = (out[base + 4] << 24) | (out[base + 5] << 16) | (out[base + 6] << 8) | out[
                                         base + 7];
                    i += 1 + pad + 4 + 4 + static_cast<size_t>(npairs) * 8;
                }
                continue;
            }

            if (op == 0xc4) {
                // wide: total length comes from the high nibble of the inner opcode
                // (0x84 iinc = 6 bytes, any other wide = 4 bytes).
                i += static_cast<size_t>(kJvmLengths[out[i + 1]] >> 4);
                continue;
            }

            if (op == 0x12 || op == 0x13 || op == 0x14) {
                // ldc / ldc_w / ldc2_w
                const int operand = (op == 0x12) ? 1 : 2;
                const int cp_index = (operand == 1) ? out[i + 1] : ((out[i + 1] << 8) | out[i + 2]);
                if (cp_index < cp->length()) {
                    const u1 tag = cp->tags()->at(cp_index);
                    if (tag == JVM_CONSTANT_String || tag == JVM_CONSTANT_MethodHandle || tag ==
                        JVM_CONSTANT_MethodType) {
                        // String/MethodHandle/MethodType constants are resolved through the
                        // resolved_references array. The JVM's rewriter converts ldc -> _fast_aldc
                        // and replaces the operand with the resolved_references index
                        // (see HotSpot rewriter.cpp, maybe_rewrite_ldc).
                        Array<u2> *ref_map = cache->reference_map();
                        const int ref_index = (ref_map != nullptr) ? ref_map->find(static_cast<u2>(cp_index)) : -1;
                        if (ref_index >= 0 && (operand > 1 || ref_index <= 0xFF)) {
                            out[i] = (operand == 1) ? FastAldc : FastAldcW;
                            if (operand == 1) {
                                out[i + 1] = static_cast<uint8_t>(ref_index & 0xFF);
                            } else {
                                out[i + 1] = static_cast<uint8_t>(ref_index & 0xFF);
                                out[i + 2] = static_cast<uint8_t>((ref_index >> 8) & 0xFF);
                            }
                        }
                    }
                    // Integer/Float/Long/Double/Class/condy ldc keep their raw CP index
                    // (the interpreter reads them straight from the constant pool).
                }
                i += 1 + operand;
                continue;
            }

            // member references (2-byte CP index). Note: new/anewarray/checkcast/instanceof
            // operands are NOT rewritten by the JVM (they stay raw CP indices).
            if (op == 0xb2 || op == 0xb3 || op == 0xb4 || op == 0xb5 ||
                op == 0xb6 || op == 0xb7 || op == 0xb8) {
                const int cp_index = (out[i + 1] << 8) | out[i + 2];
                const int cache_index = find_cache_index(cache, cp_index);
                if (cache_index >= 0) {
                    out[i + 1] = static_cast<uint8_t>(cache_index & 0xFF);
                    out[i + 2] = static_cast<uint8_t>((cache_index >> 8) & 0xFF);
                }
                i += 3;
                continue;
            }

            // invokeinterface / invokedynamic (4-byte operand, index is first u2)
            if (op == 0xb9 || op == 0xba) {
                const int cp_index = (out[i + 1] << 8) | out[i + 2];
                const int cache_index = find_cache_index(cache, cp_index);
                if (cache_index >= 0) {
                    out[i + 1] = static_cast<uint8_t>(cache_index & 0xFF);
                    out[i + 2] = static_cast<uint8_t>((cache_index >> 8) & 0xFF);
                }
                i += 5;
                continue;
            }

            i += static_cast<size_t>(instr_length(static_cast<uint8_t>(op)));
        }

        return out;
    }

    // Native (little-endian) u2 read. rewrite() stores cache/ref indices in this
    // order (low byte first), so unrewrite() must read them back the same way to be
    // a true inverse. Do NOT read them big-endian here.
    static int read_u2_le(const std::vector<uint8_t> &v, size_t pos) {
        return v[pos] | (v[pos + 1] << 8);
    }

    // Result of unrewrite(): the reconstructed class-file bytecodes plus a failure
    // signal (with bci/opcode) when an unmappable JVM-internal opcode is found.
    struct UnrewriteResult {
        std::vector<uint8_t> bytes;
        bool ok = true;
        size_t fail_bci = 0;
        uint8_t fail_opcode = 0;
    };

    // Reverse of rewrite(): converts rewritten (linked) in-memory bytecode back to
    // classfile bytecode — CP-cache indices become raw CP indices, and the JVM-internal
    // _fast_* opcodes become their plain class-file forms. Mirror of rewrite() above.
    static UnrewriteResult unrewrite(Method *method) {
        ConstantPool *cp = method->constMethod()->constant_pool();
        ConstantPoolCache *cache = cp->cache();
        std::vector<uint8_t> out(method->constMethod()->getBytecode());
        // Mirror of copy_bytecodes' `if (mh->method_holder()->is_rewritten())`:
        // is_rewritten lives on the InstanceKlass (holder), NOT on the Method
        // (decompiled copy_bytecodes reads `holder + 308 & 1`). A linked-but-not-
        // rewritten method already holds class-file operands (raw CP indices), so
        // the CP-cache / resolved-refs fixups below must NOT run -- they would
        // corrupt the indices.
        const bool rewritten = cp != nullptr && cp->pool_holder() != nullptr &&
                               cp->pool_holder()->is_rewritten();

        size_t i = 0;
        while (i < out.size()) {
            const uint8_t rawop = out[i];
            const int op = to_java_opcode(rawop);

            // tableswitch / lookupswitch (branch offsets, untouched).
            // _fast_linearswitch/_fast_binaryswitch (0xE4/0xE5) are optimized
            // lookupswitch forms whose table is already in class-file format
            // (Java u4, 4-aligned), so only the opcode needs swapping.
            if (rawop == 0xaa || rawop == 0xab || rawop == 0xe4 || rawop == 0xe5) {
                if (rawop == 0xe4 || rawop == 0xe5) {
                    out[i] = 0xab; // -> lookupswitch
                }
                size_t pad = (4 - ((i + 1) % 4)) % 4;
                if (rawop == 0xaa) {
                    size_t base = i + 1 + pad;
                    int32_t lo = (out[base + 4] << 24) | (out[base + 5] << 16) | (out[base + 6] << 8) | out[base + 7];
                    int32_t hi = (out[base + 8] << 24) | (out[base + 9] << 16) | (out[base + 10] << 8) | out[
                                     base + 11];
                    i += 1 + pad + 4 + 4 + 4 + static_cast<size_t>(hi - lo + 1) * 4;
                } else {
                    size_t base = i + 1 + pad;
                    int32_t npairs = (out[base + 4] << 24) | (out[base + 5] << 16) | (out[base + 6] << 8) | out[
                                         base + 7];
                    i += 1 + pad + 4 + 4 + static_cast<size_t>(npairs) * 8;
                }
                continue;
            }

            if (rawop == 0xc4) {
                // wide: total length comes from the high nibble of the inner opcode
                // (0x84 iinc = 6 bytes, any other wide = 4 bytes).
                i += static_cast<size_t>(kJvmLengths[out[i + 1]] >> 4);
                continue;
            }

            // _fast_aldc / _fast_aldc_w -> ldc / ldc_w. The operand holds the
            // resolved_references index; map it back to the raw CP index.
            if (rawop == FastAldc || rawop == FastAldcW) {
                const bool wide = (rawop == FastAldcW);
                const int ref_index = wide ? read_u2_le(out, i + 1) : out[i + 1];
                int cp_index = -1;
                if (rewritten && cache != nullptr) {
                    Array<u2> *ref_map = cache->reference_map();
                    if (ref_map != nullptr && ref_index >= 0 && ref_index < ref_map->length()) {
                        cp_index = ref_map->at(ref_index);
                    }
                }
                if (cp_index >= 0) {
                    if (wide) {
                        out[i] = 0x13; // ldc_w
                        out[i + 1] = static_cast<uint8_t>((cp_index >> 8) & 0xFF);
                        out[i + 2] = static_cast<uint8_t>(cp_index & 0xFF);
                    } else {
                        out[i] = 0x12; // ldc
                        out[i + 1] = static_cast<uint8_t>(cp_index & 0xFF);
                    }
                }
                i += wide ? 3 : 2;
                continue;
            }

            // member references (2-byte operand): CP-cache index -> raw CP index.
            // op is already normalized (fast_*getfield/putfield/invokevfinal -> standard),
            // so the emitted opcode is always a valid class-file one.
            if (op >= 0xb2 && op <= 0xb8) {
                int cp_index = -1;
                if (rewritten && cache != nullptr) {
                    const int cache_index = read_u2_le(out, i + 1);
                    if (cache_index >= 0 && cache_index < cache->length()) {
                        ConstantPoolCacheEntry *entry = cache->entry_at(cache_index);
                        if (entry != nullptr) {
                            cp_index = entry->constant_pool_index();
                        }
                    }
                }
                out[i] = static_cast<uint8_t>(op);
                if (cp_index >= 0) {
                    out[i + 1] = static_cast<uint8_t>((cp_index >> 8) & 0xFF);
                    out[i + 2] = static_cast<uint8_t>(cp_index & 0xFF);
                }
                i += 3;
                continue;
            }

            // invokeinterface (4-byte operand, positive u2 cache index in first two bytes)
            // and invokedynamic (native u4 operand that is the NEGATED index into the
            // resolved-indy-entries array). Mirror of copy_bytecodes' case _invokedynamic,
            // which reads `~get_native_u4(bcp+1)` and resolves through ResolvedIndyEntry.
            if (op == 0xb9 || op == 0xba) {
                int cp_index = -1;
                if (op == 0xba) {
                    if (rewritten && cache != nullptr) {
                        const uint32_t raw = static_cast<uint32_t>(out[i + 1]) |
                                             (static_cast<uint32_t>(out[i + 2]) << 8) |
                                             (static_cast<uint32_t>(out[i + 3]) << 16) |
                                             (static_cast<uint32_t>(out[i + 4]) << 24);
                        const int indy_index = static_cast<int>(~raw);
                        ResolvedIndyEntry *indy = cache->resolved_indy_entry_at(indy_index);
                        if (indy != nullptr) {
                            cp_index = indy->constant_pool_index();
                        }
                    }
                    // invokedynamic keeps zero in bytes 3-4 in the class file.
                    out[i + 3] = 0;
                    out[i + 4] = 0;
                } else {
                    if (rewritten && cache != nullptr) {
                        const int cache_index = read_u2_le(out, i + 1);
                        if (cache_index >= 0 && cache_index < cache->length()) {
                            ConstantPoolCacheEntry *entry = cache->entry_at(cache_index);
                            if (entry != nullptr) {
                                cp_index = entry->constant_pool_index();
                            }
                        }
                    }
                }
                if (cp_index >= 0) {
                    out[i + 1] = static_cast<uint8_t>((cp_index >> 8) & 0xFF);
                    out[i + 2] = static_cast<uint8_t>(cp_index & 0xFF);
                }
                i += 5;
                continue;
            }

            if (rawop >= 0xCB && op == rawop) {
                // Reached an unmapped JVM-internal opcode (0xCB+ are never valid
                // class-file opcodes). Refuse to emit garbage bytes.
                return UnrewriteResult{{}, false, i, rawop};
            }
            out[i] = static_cast<uint8_t>(op);
            i += static_cast<size_t>(instr_length(static_cast<uint8_t>(op)));
        }

        return UnrewriteResult{std::move(out), true, 0, 0};
    }
} // namespace BytecodeRewriter
