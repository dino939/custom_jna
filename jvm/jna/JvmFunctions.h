//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_JVMFUNCTIONS_H
#define CUSTOM_JNA_JVMFUNCTIONS_H
#include <cstdint>
#include <vector>
#include "../struct/oop/klass/InstanceKlass.h"
#include "../types.h"
#include "../utils/assert/assert.hpp"
#include "../struct/oop/classloader/ClassLoaderData.h"

class JvmFunctions
{
public:
    static InstanceKlass* findKlass17JVM(const char* name);
    static bool redefineClass(InstanceKlass* klass, const std::vector<uint8_t>& new_class_bytes);
    static std::vector<uint8_t> reconstructClass(InstanceKlass* klass);
    static fieldID find_local_field_id(InstanceKlass* klass, const char* fieldName, const char* fieldDesc);
    static Method* findMethod(InstanceKlass *klass, const char *name, const char *signature);
    static object getObjectField(object oop, fieldID fieldID);
    static jboolean getBooleanField(object oop, fieldID id);
    static jbyte getByteField(object oop, fieldID id);
    static jchar getCharField(object oop, fieldID id);
    static jshort getShortField(object oop, fieldID id);
    static jint getIntField(object oop, fieldID id);
    static jlong getLongField(object oop, fieldID id);
    static jfloat getFloatField(object oop, fieldID id);
    static jdouble getDoubleField(object oop, fieldID id);
    static void setObjectField(object oop, fieldID fieldID, object value);
    static void setBooleanField(object oop, fieldID fieldID, jboolean value);
    static void setByteField(object oop, fieldID fieldID, jbyte value);
    static void setCharField(object oop, fieldID fieldID, jchar value);
    static void setShortField(object oop, fieldID fieldID, jshort value);
    static void setIntField(object oop, fieldID fieldID, jint value);
    static void setLongField(object oop, fieldID fieldID, jlong value);
    static void setFloatField(object oop, fieldID fieldID, jfloat value);
    static void setDoubleField(object oop, fieldID fieldID, jdouble value);
    static object getStaticObjectField(InstanceKlass* clazz, fieldID fieldID);
    static jboolean getStaticBooleanField(InstanceKlass* clazz, fieldID fieldID);
    static jbyte getStaticByteField(InstanceKlass* clazz, fieldID fieldID);
    static jchar getStaticCharField(InstanceKlass* clazz, fieldID fieldID);
    static jshort getStaticShortField(InstanceKlass* clazz, fieldID fieldID);
    static jint getStaticIntField(InstanceKlass* clazz, fieldID fieldID);
    static jlong getStaticLongField(InstanceKlass* clazz, fieldID fieldID);
    static jfloat getStaticFloatField(InstanceKlass* clazz, fieldID fieldID);
    static jdouble getStaticDoubleField(InstanceKlass* clazz, fieldID fieldID);
    static void setStaticObjectField(InstanceKlass* clazz, fieldID fieldID, object value);
    static void setStaticBooleanField(InstanceKlass* clazz, fieldID fieldID, jboolean value);
    static void setStaticByteField(InstanceKlass* clazz, fieldID fieldID, jbyte value);
    static void setStaticCharField(InstanceKlass* clazz, fieldID fieldID, jchar value);
    static void setStaticShortField(InstanceKlass* clazz, fieldID fieldID, jshort value);
    static void setStaticIntField(InstanceKlass* clazz, fieldID fieldID, jint value);
    static void setStaticLongField(InstanceKlass* clazz, fieldID fieldID, jlong value);
    static void setStaticFloatField(InstanceKlass* clazz, fieldID fieldID, jfloat value);
    static void setStaticDoubleField(InstanceKlass* clazz, fieldID fieldID, jdouble value);
    static jboolean isEnabledKlassBytecodeVerification();
    static void setKlassBytecodeVerification(jboolean state);
};


#endif //CUSTOM_JNA_JVMFUNCTIONS_H
