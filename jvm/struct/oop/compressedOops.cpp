//
// Created by prone on 30.06.2026.
//

#include "compressedOops.h"
#include "../../offset/OffsetExtractor.h"

bool* CompressedOops::use_implicit_null_checks = static_cast<bool*>(OffsetExtractor::getInstance()->findVMStructEntry("CompressedOops", "_narrow_oop._use_implicit_null_checks")->address);

void* CompressedOops::base = *static_cast<void**>(OffsetExtractor::getInstance()->findVMStructEntry("CompressedOops", "_narrow_oop._base")->address);

jint CompressedOops::shift = *static_cast<jint*>(OffsetExtractor::getInstance()->findVMStructEntry("CompressedOops", "_narrow_oop._shift")->address);

bool CompressedOops::isCompressedOops()
{
    assert(use_implicit_null_checks != nullptr, "use_implicit_null_checks is null");
    return *use_implicit_null_checks;
}

object CompressedOops::decodeOop(object oop)
{
    if (!isCompressedOops() || oop == nullptr) return oop;

    auto localOop = reinterpret_cast<uintptr_t>(oop);
        localOop <<= shift;
        localOop += reinterpret_cast<uintptr_t>(base);

    return reinterpret_cast<object>(localOop);
}

object CompressedOops::encodeOop(object oop)
{
    if (!isCompressedOops() || oop == nullptr) return oop;

    auto localOop = reinterpret_cast<uintptr_t>(oop);
        localOop -= reinterpret_cast<uintptr_t>(base);
        localOop >>= shift;
    return reinterpret_cast<object>(localOop);
}
