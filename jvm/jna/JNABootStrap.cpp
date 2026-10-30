//
// Created by prone on 17.03.2026.
//

#include "JNABootStrap.h"
#include "JvmFunctions.h"
#include "../jni/JniJnaConnector.h"

JNABootStrap *JNABootStrap::instance = nullptr;

JNABootStrap::JNABootStrap() : env(new JNINativeAccess_) {
    env->findClass = &JvmFunctions::findKlass17JVM;
    env->findFieldId = &JvmFunctions::find_local_field_id;
    env->findMethod = &JvmFunctions::findMethod;
    //non-static get
    env->getObjectField = &JvmFunctions::getObjectField;
    env->getBooleanField = &JvmFunctions::getBooleanField;
    env->getByteField = &JvmFunctions::getByteField;
    env->getCharField = &JvmFunctions::getCharField;
    env->getShortField = &JvmFunctions::getShortField;
    env->getIntField = &JvmFunctions::getIntField;
    env->getLongField = &JvmFunctions::getLongField;
    env->getFloatField = &JvmFunctions::getFloatField;
    env->getDoubleField = &JvmFunctions::getDoubleField;
    //non-static set
    env->setObjectField = &JvmFunctions::setObjectField;
    env->setBooleanField = &JvmFunctions::setBooleanField;
    env->setByteField = &JvmFunctions::setByteField;
    env->setCharField = &JvmFunctions::setCharField;
    env->setShortField = &JvmFunctions::setShortField;
    env->setIntField = &JvmFunctions::setIntField;
    env->setLongField = &JvmFunctions::setLongField;
    env->setFloatField = &JvmFunctions::setFloatField;
    env->setDoubleField = &JvmFunctions::setDoubleField;
    //static get
    env->getStaticObjectField = &JvmFunctions::getStaticObjectField;
    env->getStaticBooleanField = &JvmFunctions::getStaticBooleanField;
    env->getStaticByteField = &JvmFunctions::getStaticByteField;
    env->getStaticCharField = &JvmFunctions::getStaticCharField;
    env->getStaticShortField = &JvmFunctions::getStaticShortField;
    env->getStaticIntField = &JvmFunctions::getStaticIntField;
    env->getStaticLongField = &JvmFunctions::getStaticLongField;
    env->getStaticFloatField = &JvmFunctions::getStaticFloatField;
    env->getStaticDoubleField = &JvmFunctions::getStaticDoubleField;
    //static set
    env->setStaticObjectField = &JvmFunctions::setStaticObjectField;
    env->setStaticBooleanField = &JvmFunctions::setStaticBooleanField;
    env->setStaticByteField = &JvmFunctions::setStaticByteField;
    env->setStaticCharField = &JvmFunctions::setStaticCharField;
    env->setStaticShortField = &JvmFunctions::setStaticShortField;
    env->setStaticIntField = &JvmFunctions::setStaticIntField;
    env->setStaticLongField = &JvmFunctions::setStaticLongField;
    env->setStaticFloatField = &JvmFunctions::setStaticFloatField;
    env->setStaticDoubleField = &JvmFunctions::setStaticDoubleField;

    //flags
    env->isEnabledKlassBytecodeVerification = &JvmFunctions::isEnabledKlassBytecodeVerification;
    env->setKlassBytecodeVerification = &JvmFunctions::setKlassBytecodeVerification;

    //redefine
    env->redefineClass = &JvmFunctions::redefineClass;

    //reconstruct
    env->reconstructClass = &JvmFunctions::reconstructClass;

    //jni<->jna connector
    env->jclassToInstanceKlass = &JniJnaConnector::toInstanceKlass;
    env->instanceKlassToJClass = &JniJnaConnector::toJClass;
    env->deleteJClassRef = &JniJnaConnector::deleteRef;
}

JNABootStrap::~JNABootStrap() {
    delete env;
}

environment JNABootStrap::getEnv() const {
    return env;
}
