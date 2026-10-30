//
// Created by prone on 04.08.2026.
//

#include "JavaThread.h"

#include <windows.h>

#include "../../offset/OffsetExtractor.h"

namespace {
    constexpr uint64_t kNotFoundOffset = static_cast<uint64_t>(-1);
    typedef jint (__stdcall *JNI_GetCreatedJavaVMs_Fn)(JavaVM_** vmBuf, jsize bufLen, jsize* nVMs);


}

JavaThread* JavaThread::CurrentThread() {
    OffsetExtractor* offsets = OffsetExtractor::getInstance();
    VMStructEntry* anchor_entry = offsets->findVMStructEntry("JavaThread", "_anchor");
    VMTypeEntry* anchor_type = offsets->findVMTypeEntry("JavaFrameAnchor");
    if (anchor_entry == nullptr || anchor_entry->offset == kNotFoundOffset || anchor_type == nullptr) {
        return nullptr;
    }

    HMODULE jvm = GetModuleHandleA("jvm.dll");
    if (jvm == nullptr) {
        return nullptr;
    }
    auto get_created = reinterpret_cast<JNI_GetCreatedJavaVMs_Fn>(GetProcAddress(jvm, "JNI_GetCreatedJavaVMs"));
    if (get_created == nullptr) {
        return nullptr;
    }
    JavaVM_* vm = nullptr;
    jsize count = 0;
    if (get_created(&vm, 1, &count) != 0 || count < 1 || vm == nullptr) {
        return nullptr;
    }
    void* env = nullptr;
    if (vm->functions->GetEnv(vm, &env, JNI_VERSION_1_6) != 0 || env == nullptr) {
        return nullptr;
    }

    uintptr_t thread = reinterpret_cast<uintptr_t>(env) -
                       (anchor_entry->offset + anchor_type->size + 8);
    return reinterpret_cast<JavaThread*>(thread);
}

int JavaThread::getThreadState() {
    VMStructEntry* state_entry = OffsetExtractor::getInstance()->findVMStructEntry("JavaThread", "_thread_state");
    if (state_entry == nullptr || state_entry->offset == kNotFoundOffset) {
        return 0;
    }
    return *reinterpret_cast<int*>(reinterpret_cast<uint8_t*>(this) + state_entry->offset);
}

void JavaThread::setThreadState(int state) {
    VMStructEntry* state_entry = OffsetExtractor::getInstance()->findVMStructEntry("JavaThread", "_thread_state");
    if (state_entry == nullptr || state_entry->offset == kNotFoundOffset) {
        return;
    }
    *reinterpret_cast<int*>(reinterpret_cast<uint8_t*>(this) + state_entry->offset) = state;
}