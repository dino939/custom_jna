#include <windows.h>

#include "jvm/jna/JNABootStrap.h"
#include "jvm/jna.h"
#include "jvm/jni/JniJnaConnector.h"

static environment jnaEnv;

jbyteArray klassReconstruct(JNIEnv *jniEnv, jclass callableClass, jclass klazz) {
    std::vector<uint8_t> bytes = jnaEnv->reconstructClass(JniJnaConnector::toInstanceKlass(klazz));
    jbyteArray result = jniEnv->NewByteArray(static_cast<jsize>(bytes.size()));
    jniEnv->SetByteArrayRegion(result, 0, static_cast<jsize>(bytes.size()),
                               reinterpret_cast<const jbyte *>(bytes.data()));
    return result;
}

jboolean klassRedefine(JNIEnv *jniEnv, jclass callableClass, jclass klazz, jbyteArray newBytes) {
    jsize len = jniEnv->GetArrayLength(newBytes);
    std::vector<uint8_t> bytes(static_cast<size_t>(len));
    jniEnv->GetByteArrayRegion(newBytes, 0, len, reinterpret_cast<jbyte *>(bytes.data()));
    bool ok = jnaEnv->redefineClass(JniJnaConnector::toInstanceKlass(klazz), bytes);
    return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *jniEnv;
    vm->GetEnv((void **) &jniEnv, JNI_VERSION_1_8);
    JniJnaConnector::init(vm);
    jnaEnv = JNABootStrap::getInstance()->getEnv();
    jclass klass = jniEnv->FindClass("das/ist/denger/systems/jvm/JvmInterface");
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
    }
    if (klass == nullptr) {
        klass = jniEnv->FindClass("org/example/Main");
        if (jniEnv->ExceptionCheck()) {
            jniEnv->ExceptionClear();
        }
    }

    JNINativeMethod methods[] = {
        {(char *) "klassReconstruct", (char *) "(Ljava/lang/Class;)[B", reinterpret_cast<void *>(klassReconstruct)},
        {(char *) "klassRedefine", (char *) "(Ljava/lang/Class;[B)Z", reinterpret_cast<void *>(klassRedefine)}
    };

    jint res = klass != nullptr ? jniEnv->RegisterNatives(klass, methods, 2) : JNI_ERR;

    return JNI_VERSION_1_8;
}
