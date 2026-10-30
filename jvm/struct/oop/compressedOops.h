//
// Created by prone on 30.06.2026.
//

#ifndef CUSTOM_JNA_COMPRESSEDOOPS_H
#define CUSTOM_JNA_COMPRESSEDOOPS_H
#include "../../types.h"


class CompressedOops
{
    static bool* use_implicit_null_checks;

public:
    static void* base;
    static jint shift;
    static bool isCompressedOops();
    static object decodeOop(object oop);
    static object encodeOop(object oop);
};


#endif //CUSTOM_JNA_COMPRESSEDOOPS_H