//
// Created by prone on 04.08.2026.
//

#include "JniJnaConnector.h"
#include <mutex>
#include "../offset/OffsetExtractor.h"
#include "../struct/oop/handle/OopHandle.h"

static JavaVM *gJavaVM = nullptr;

void JniJnaConnector::init(JavaVM *vm) {
    gJavaVM = vm;
}

static JNIEnv *getJniEnv() {
    static JNIEnv *env = nullptr;
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (env == nullptr && gJavaVM != nullptr) {
        if (gJavaVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8) != JNI_OK) {
            if (gJavaVM->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr) != JNI_OK) {
                env = nullptr;
            }
        }
    }
    return env;
}

InstanceKlass *JniJnaConnector::toInstanceKlass(jclass clazz) {
    if (clazz == nullptr) {
        return nullptr;
    }

    object mirror = *reinterpret_cast<object *>(clazz);
    if (mirror == nullptr) {
        return nullptr;
    }

    static VMStructEntry *entry = OffsetExtractor::getInstance()->findVMStructEntry("java_lang_Class", "_klass_offset");
    if (entry->address == nullptr) {
        return nullptr;
    }
    static int offset = *static_cast<int *>(entry->address);
    return *reinterpret_cast<InstanceKlass**>(reinterpret_cast<uintptr_t>(mirror) + offset);
//    return static_cast<InstanceKlass*>(mirror + offset);
}

jclass JniJnaConnector::toJClass(InstanceKlass *klass) {
    if (klass == nullptr) {
        return nullptr;
    }
    OopHandle mirror = klass->java_mirror();
    if (mirror.is_empty()) {
        return nullptr;
    }
    return reinterpret_cast<jclass>(mirror.get_ptr());
}

void JniJnaConnector::deleteRef(jclass clazz) {
    if (clazz == nullptr) {
        return;
    }
    JNIEnv *env = getJniEnv();
    if (env != nullptr) {
        env->DeleteGlobalRef(clazz);
    }
}
