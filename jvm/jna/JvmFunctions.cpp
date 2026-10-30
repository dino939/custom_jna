//
// Created by prone on 17.03.2026.
//

#include "JvmFunctions.h"
#include <string>
#include <unordered_map>

#include "../struct/oop/constantPool/ConstantPool.h"
#include "../struct/oop/compressedOops.h"
#include "../struct/oop/method/method.h"
#include "../utils/fields/FieldInfoReader.hpp"
#include "../redefine/classRedefiner.h"
#include "../reconstruct/classFileReconstituter.h"

static std::unordered_map<std::string, InstanceKlass *> klasses;

InstanceKlass *JvmFunctions::findKlass17JVM(const char *name) {
    if (name == nullptr) {
        return nullptr;
    }

    const auto cached = klasses.find(name);
    if (cached != klasses.end()) {
        return cached->second;
    }

    for (ClassLoaderData *classLoaderData = ClassLoaderData::_head();
         classLoaderData != nullptr;
         classLoaderData = classLoaderData->_next()) {
        for (InstanceKlass *klass = classLoaderData->_klasses();
             klass != nullptr;
             klass = klass->next()) {
            assert(klass->name() != nullptr, "class has null name symbol");
            Symbol *className = klass->name();
            if (className != nullptr && strcmp(name, className->as_C_string()) == 0) {
                klasses[name] = klass;
                return klass;
            }
        }
    }

    return nullptr;
}

fieldID JvmFunctions::find_local_field_id(InstanceKlass *klass, const char *fieldName, const char *fieldDesc) {
    Array<u1> *stream = klass->fieldinfo_stream();

    if (stream != nullptr) {
        FieldInfoReader reader(stream);

        int num_java = reader.next_unit();
        int num_injected = reader.next_unit();
        int total_fields = num_java + num_injected;

        FieldInfo_20 fi;

        for (int i = 0; i < total_fields; i++) {
            reader.read_field_info(fi);

            Symbol *_name = klass->constants()->symbol_at(fi.name_index());
            Symbol *_sign = klass->constants()->symbol_at(fi.signature_index());

            if (strcmp(_name->as_C_string(), fieldName) == 0 &&
                strcmp(_sign->as_C_string(), fieldDesc) == 0) {
                return fi.offset();
            }
        }
    } else {
        for (int i = 0; i < (klass->fields()->length() / 6); i++) {
            FieldInfo_17 *field = FieldInfo_17::from_field_array(klass->fields(), i);
            if (strcmp(klass->constants()->symbol_at(field->name_index())->as_C_string(), fieldName) == 0 &&
                strcmp(klass->constants()->symbol_at(field->signature_index())->as_C_string(), fieldDesc) == 0)
                return field->offset();
        };
    }

    return 0;
}


Method *JvmFunctions::findMethod(InstanceKlass *klass, const char *name, const char *signature) {
    assert(klass != nullptr, "klass is null");
    assert(name != nullptr, "method name is null");
    assert(signature != nullptr, "method signature is null");
    ConstantPool *cp = klass->constants();
    assert(cp != nullptr, "class has null constant pool");
    Array<Method *> *methods = klass->methods();
    assert(methods != nullptr, "class has null methods array");
    for (int i = 0; i < methods->length(); i++) {
        if (Method *method = methods->at(i);
            strcmp(cp->symbol_at(method->constMethod()->name_index())->as_C_string(), name) == 0 &&
            strcmp(cp->symbol_at(method->constMethod()->signature_index())->as_C_string(), signature) == 0) {
            return method;
        }
    }
    return nullptr;
}

template<typename T>
T getGenericField(object oop, const fieldID fieldID) {
    if (!fieldID) return T{};
    return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(oop) + fieldID);
}

template<typename T>
void setGenericField(object oop, fieldID fieldID, T value) {
    if (!fieldID) return;
    *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(oop) + fieldID) = value;
}

template<typename T>
static T getStaticGenericField(InstanceKlass *klass, const fieldID fieldID) {
    return getGenericField<T>(klass->java_mirror().get_oop(), fieldID);
}

template<typename T>
static void setStaticGenericField(InstanceKlass *klass, fieldID fieldID, T value) {
    if (!fieldID || !klass) return;
    setGenericField<T>(klass->java_mirror().get_oop(), fieldID, value);
}

object JvmFunctions::getObjectField(object oop, const fieldID fieldID) {
    if (!fieldID || !oop) return nullptr;
    const auto value = reinterpret_cast<object>(getGenericField<unsigned int>(oop, fieldID));
    return CompressedOops::decodeOop(value);
}

jboolean JvmFunctions::getBooleanField(object oop, const fieldID fieldID) {
    return getGenericField<jboolean>(oop, fieldID);
}

jbyte JvmFunctions::getByteField(object oop, const fieldID fieldID) {
    return getGenericField<jbyte>(oop, fieldID);
}

jchar JvmFunctions::getCharField(object oop, const fieldID fieldID) {
    return getGenericField<jchar>(oop, fieldID);
}

jshort JvmFunctions::getShortField(object oop, const fieldID fieldID) {
    return getGenericField<jshort>(oop, fieldID);
}

jint JvmFunctions::getIntField(object oop, const fieldID fieldID) {
    return getGenericField<jint>(oop, fieldID);
}

jlong JvmFunctions::getLongField(object oop, const fieldID fieldID) {
    return getGenericField<jlong>(oop, fieldID);
}

jfloat JvmFunctions::getFloatField(object oop, const fieldID fieldID) {
    return getGenericField<jfloat>(oop, fieldID);
}

jdouble JvmFunctions::getDoubleField(object oop, const fieldID fieldID) {
    return getGenericField<jdouble>(oop, fieldID);
}

void JvmFunctions::setObjectField(object oop, fieldID fieldID, object value) {
    value = CompressedOops::encodeOop(value);
    uint32_t compressed = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value));
    setGenericField<uint32_t>(oop, fieldID, compressed);
}

void JvmFunctions::setBooleanField(object oop, fieldID fieldID, jboolean value) {
    setGenericField<jboolean>(oop, fieldID, value);
}

void JvmFunctions::setByteField(object oop, fieldID fieldID, jbyte value) {
    setGenericField<jbyte>(oop, fieldID, value);
}

void JvmFunctions::setCharField(object oop, fieldID fieldID, jchar value) {
    setGenericField<jchar>(oop, fieldID, value);
}

void JvmFunctions::setShortField(object oop, fieldID fieldID, jshort value) {
    setGenericField<jshort>(oop, fieldID, value);
}

void JvmFunctions::setIntField(object oop, fieldID fieldID, jint value) {
    setGenericField<jint>(oop, fieldID, value);
}

void JvmFunctions::setLongField(object oop, fieldID fieldID, jlong value) {
    setGenericField<jlong>(oop, fieldID, value);
}

void JvmFunctions::setFloatField(object oop, fieldID fieldID, jfloat value) {
    setGenericField<jfloat>(oop, fieldID, value);
}

void JvmFunctions::setDoubleField(object oop, fieldID fieldID, jdouble value) {
    setGenericField<jdouble>(oop, fieldID, value);
}

object JvmFunctions::getStaticObjectField(InstanceKlass *clazz, fieldID fieldID) {
    return getObjectField(clazz->java_mirror().get_oop(), fieldID);
}

jboolean JvmFunctions::getStaticBooleanField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jboolean>(clazz, fieldID);
}

jbyte JvmFunctions::getStaticByteField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jbyte>(clazz, fieldID);
}

jchar JvmFunctions::getStaticCharField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jchar>(clazz, fieldID);
}

jshort JvmFunctions::getStaticShortField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jshort>(clazz, fieldID);
}

jint JvmFunctions::getStaticIntField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jint>(clazz, fieldID);
}

jlong JvmFunctions::getStaticLongField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jlong>(clazz, fieldID);
}

jfloat JvmFunctions::getStaticFloatField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jfloat>(clazz, fieldID);
}

jdouble JvmFunctions::getStaticDoubleField(InstanceKlass *clazz, fieldID fieldID) {
    return getStaticGenericField<jdouble>(clazz, fieldID);
}

void JvmFunctions::setStaticObjectField(InstanceKlass *clazz, fieldID fieldID, object value) {
    setObjectField(clazz->java_mirror().get_oop(), fieldID, value);
}

void JvmFunctions::setStaticBooleanField(InstanceKlass *clazz, fieldID fieldID, jboolean value) {
    setStaticGenericField<jboolean>(clazz, fieldID, value);
}

void JvmFunctions::setStaticByteField(InstanceKlass *clazz, fieldID fieldID, jbyte value) {
    setStaticGenericField<jbyte>(clazz, fieldID, value);
}

void JvmFunctions::setStaticCharField(InstanceKlass *clazz, fieldID fieldID, jchar value) {
    setStaticGenericField<jchar>(clazz, fieldID, value);
}

void JvmFunctions::setStaticShortField(InstanceKlass *clazz, fieldID fieldID, jshort value) {
    setStaticGenericField<jshort>(clazz, fieldID, value);
}

void JvmFunctions::setStaticIntField(InstanceKlass *clazz, fieldID fieldID, jint value) {
    setStaticGenericField<jint>(clazz, fieldID, value);
}

void JvmFunctions::setStaticLongField(InstanceKlass *clazz, fieldID fieldID, jlong value) {
    setStaticGenericField<jlong>(clazz, fieldID, value);
}

void JvmFunctions::setStaticFloatField(InstanceKlass *clazz, fieldID fieldID, jfloat value) {
    setStaticGenericField<jfloat>(clazz, fieldID, value);
}

void JvmFunctions::setStaticDoubleField(InstanceKlass *clazz, fieldID fieldID, jdouble value) {
    setStaticGenericField<jdouble>(clazz, fieldID, value);
}

jboolean JvmFunctions::isEnabledKlassBytecodeVerification() {
    static JVMFlag *flag = OffsetExtractor::getInstance()->findJVMFlag("BytecodeVerificationRemote");
    return *static_cast<bool *>(flag->_addr);
}

void JvmFunctions::setKlassBytecodeVerification(jboolean state) {
    static JVMFlag *flag = OffsetExtractor::getInstance()->findJVMFlag("BytecodeVerificationRemote");
    *static_cast<bool *>(flag->_addr) = state;
}

bool JvmFunctions::redefineClass(InstanceKlass *klass, const std::vector<uint8_t> &new_class_bytes) {
    assert(klass != nullptr, "klass is null");
    assert(!new_class_bytes.empty(), "new class bytes are empty");
    return redefine::redefine_class(klass, new_class_bytes);
}

std::vector<uint8_t> JvmFunctions::reconstructClass(InstanceKlass *klass) {
    assert(klass != nullptr, "klass is null");
    ClassFileReconstituter reconstituter(klass);
    return reconstituter.bytes();
}
