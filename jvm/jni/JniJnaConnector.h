//
// Created by prone on 04.08.2026.
//

#ifndef CUSTOM_JNI_JNIJNACONNECTOR_H
#define CUSTOM_JNI_JNIJNACONNECTOR_H

#include "../types.h"
#include "../struct/oop/klass/InstanceKlass.h"

class JniJnaConnector
{
public:
    static void init(JavaVM* vm);
    static InstanceKlass* toInstanceKlass(jclass clazz);
    static jclass toJClass(InstanceKlass* klass);
    static void deleteRef(jclass clazz);
};

#endif //CUSTOM_JNI_JNIJNACONNECTOR_H
