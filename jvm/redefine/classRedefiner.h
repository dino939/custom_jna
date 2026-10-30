//
// Created by prone on 04.08.2026.
//

#ifndef CUSTOM_JNA_CLASSREDEFINER_H
#define CUSTOM_JNA_CLASSREDEFINER_H

#include <cstdint>
#include <string>
#include <vector>
#include "../struct/oop/klass/InstanceKlass.h"
#include "../struct/oop/constantPool/ConstantPool.h"
#include "../struct/oop/method/exceptionTableElement.h"

namespace redefine {

struct RedefineResult {
    bool success = false;
    std::string error;
    ConstantPool* new_cp = nullptr;
    std::vector<int> index_map; // new class CP index -> merged CP index
};

/**
 * Merge the constant pool from new class bytes into the existing ConstantPool
 * of an already-loaded class.  Returns a new (larger) ConstantPool and an
 * index map that translates raw CP indices from the new class file into
 * indices in the merged pool.
 *
 * The original ConstantPool is left untouched; callers must install new_cp
 * on the class (and rebuild the ConstantPoolCache) afterwards.
 */
RedefineResult merge_constant_pool(ConstantPool* old_cp,
                                     const std::vector<uint8_t>& new_class_bytes);

/**
 * Rewrite raw constant-pool indices inside bytecode so they point into the
 * merged constant pool.  Handles ldc -> ldc_w widening when a merged index
 * no longer fits in one byte.
 */
bool rewrite_bytecode_cp_indices(std::vector<uint8_t>& bytecode,
                                 const std::vector<int>& index_map,
                                 std::vector<ExceptionTableElement>* exception_table = nullptr);

/**
 * Reverse the bytecode rewriting that the HotSpot Rewriter performed for the
 * currently installed ConstantPoolCache, restoring raw CP indices.
 * This must be done on all methods of a class before calling the native
 * Rewriter::rewrite() on that class.
 */
bool restore_method_bytecodes(Method* method, ConstantPool* cp);

/**
 * Find and call the native HotSpot Rewriter::rewrite(InstanceKlass*, Thread*).
 *
 * The address is located with the pattern scanner (scanner.hpp).  Replace
 * kRewriterPattern with the correct byte pattern for your JDK build.
 */
bool invoke_native_rewriter(InstanceKlass* ik);

/**
 * High-level helper: replace a single method's bytecode with the bytecode of
 * the same-named method from the new class bytes, merge the constant pools,
 * and rebuild the ConstantPoolCache.
 */
bool redefine_method(InstanceKlass* ik,
                     const char* method_name,
                     const char* method_signature,
                     const std::vector<uint8_t>& new_class_bytes);

/**
 * Redefine all methods of a class whose name+signature exist in both the
 * old class and the new class bytes. Merges the constant pool once and
 * rebuilds the ConstantPoolCache once.
 */
bool redefine_class(InstanceKlass* ik,
                    const std::vector<uint8_t>& new_class_bytes);

} // namespace redefine

#endif //CUSTOM_JNA_CLASSREDEFINER_H
