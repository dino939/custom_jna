//
// Created by prone on 17.03.2026.
//

#include "JNABootStrap.h"
#include "JvmFunctions.h"

JNABootStrap* JNABootStrap::instance = nullptr;

JNABootStrap::JNABootStrap() : env(new JNINativeAccess_) {
    env->findClass = &JvmFunctions::findKlass17JVM;
}

JNABootStrap::~JNABootStrap() {
    delete env;
}

environment JNABootStrap::_env() const
{
    return env;
}
