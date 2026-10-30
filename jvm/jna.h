//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_JNA_H
#define CUSTOM_JNA_JNA_H
#include <vector>
#include "types.h"

typedef InstanceKlass * (*FindCLassPtr)(const char *name);

typedef fieldID (*FindField)(InstanceKlass *klass, const char *fieldName, const char *fieldDesc);

typedef Method* (*FindMethod)(InstanceKlass *klass, const char *name, const char *signature);

typedef jboolean (*IsEnabledKlassBytecodeVerification)();

typedef void (*SetKlassBytecodeVerification)(jboolean state);

typedef object (*GetObjectField)(object oop, fieldID fieldID);

typedef jboolean (*GetBooleanField)(object oop, fieldID fieldID);

typedef jbyte (*GetByteField)(object oop, fieldID fieldID);

typedef jchar (*GetCharField)(object oop, fieldID fieldID);

typedef jshort (*GetShortField)(object oop, fieldID fieldID);

typedef jint (*GetIntField)(object oop, fieldID fieldID);

typedef jlong (*GetLongField)(object oop, fieldID fieldID);

typedef jfloat (*GetFloatField)(object oop, fieldID fieldID);

typedef jdouble (*GetDoubleField)(object oop, fieldID fieldID);

typedef void (*SetObjectField)(object oop, fieldID fieldID, object value);

typedef void (*SetBooleanField)(object oop, fieldID fieldID, jboolean value);

typedef void (*SetByteField)(object oop, fieldID fieldID, jbyte value);

typedef void (*SetCharField)(object oop, fieldID fieldID, jchar value);

typedef void (*SetShortField)(object oop, fieldID fieldID, jshort value);

typedef void (*SetIntField)(object oop, fieldID fieldID, jint value);

typedef void (*SetLongField)(object oop, fieldID fieldID, jlong value);

typedef void (*SetFloatField)(object oop, fieldID fieldID, jfloat value);

typedef void (*SetDoubleField)(object oop, fieldID fieldID, jdouble value);

typedef object (*GetStaticObjectField)(InstanceKlass *clazz, fieldID fieldID);

typedef jboolean (*GetStaticBooleanField)(InstanceKlass *clazz, fieldID fieldID);

typedef jbyte (*GetStaticByteField)(InstanceKlass *clazz, fieldID fieldID);

typedef jchar (*GetStaticCharField)(InstanceKlass *clazz, fieldID fieldID);

typedef jshort (*GetStaticShortField)(InstanceKlass *clazz, fieldID fieldID);

typedef jint (*GetStaticIntField)(InstanceKlass *clazz, fieldID fieldID);

typedef jlong (*GetStaticLongField)(InstanceKlass *clazz, fieldID fieldID);

typedef jfloat (*GetStaticFloatField)(InstanceKlass *clazz, fieldID fieldID);

typedef jdouble (*GetStaticDoubleField)(InstanceKlass *clazz, fieldID fieldID);

typedef void (*SetStaticObjectField)(InstanceKlass *clazz, fieldID fieldID, object value);

typedef void (*SetStaticBooleanField)(InstanceKlass *clazz, fieldID fieldID, jboolean value);

typedef void (*SetStaticByteField)(InstanceKlass *clazz, fieldID fieldID, jbyte value);

typedef void (*SetStaticCharField)(InstanceKlass *clazz, fieldID fieldID, jchar value);

typedef void (*SetStaticShortField)(InstanceKlass *clazz, fieldID fieldID, jshort value);

typedef void (*SetStaticIntField)(InstanceKlass *clazz, fieldID fieldID, jint value);

typedef void (*SetStaticLongField)(InstanceKlass *clazz, fieldID fieldID, jlong value);

typedef void (*SetStaticFloatField)(InstanceKlass *clazz, fieldID fieldID, jfloat value);

typedef void (*SetStaticDoubleField)(InstanceKlass *clazz, fieldID fieldID, jdouble value);

typedef bool (*RedefineClassPtr)(InstanceKlass *klass, const std::vector<uint8_t> &new_class_bytes);

typedef std::vector<uint8_t> (*ReconstructClassPtr)(InstanceKlass *klass);

typedef InstanceKlass* (*JClassToInstanceKlassPtr)(jclass clazz);

typedef jclass (*InstanceKlassToJClassPtr)(InstanceKlass *klass);

typedef void (*DeleteJClassRefPtr)(jclass clazz);


struct JNINativeAccess_ {
    FindCLassPtr findClass;
    FindField findFieldId;
    FindMethod findMethod;
    GetObjectField getObjectField;
    GetBooleanField getBooleanField;
    GetByteField getByteField;
    GetCharField getCharField;
    GetShortField getShortField;
    GetIntField getIntField;
    GetLongField getLongField;
    GetFloatField getFloatField;
    GetDoubleField getDoubleField;
    SetObjectField setObjectField;
    SetBooleanField setBooleanField;
    SetByteField setByteField;
    SetCharField setCharField;
    SetShortField setShortField;
    SetIntField setIntField;
    SetLongField setLongField;
    SetFloatField setFloatField;
    SetDoubleField setDoubleField;
    GetStaticObjectField getStaticObjectField;
    GetStaticBooleanField getStaticBooleanField;
    GetStaticByteField getStaticByteField;
    GetStaticCharField getStaticCharField;
    GetStaticShortField getStaticShortField;
    GetStaticIntField getStaticIntField;
    GetStaticLongField getStaticLongField;
    GetStaticFloatField getStaticFloatField;
    GetStaticDoubleField getStaticDoubleField;
    SetStaticObjectField setStaticObjectField;
    SetStaticBooleanField setStaticBooleanField;
    SetStaticByteField setStaticByteField;
    SetStaticCharField setStaticCharField;
    SetStaticShortField setStaticShortField;
    SetStaticIntField setStaticIntField;
    SetStaticLongField setStaticLongField;
    SetStaticFloatField setStaticFloatField;
    SetStaticDoubleField setStaticDoubleField;
    IsEnabledKlassBytecodeVerification isEnabledKlassBytecodeVerification;
    SetKlassBytecodeVerification setKlassBytecodeVerification;
    RedefineClassPtr redefineClass;
    ReconstructClassPtr reconstructClass;
    JClassToInstanceKlassPtr jclassToInstanceKlass;
    InstanceKlassToJClassPtr instanceKlassToJClass;
    DeleteJClassRefPtr deleteJClassRef;
};

typedef JNINativeAccess_ *environment;

#endif //CUSTOM_JNA_JNA_H
