//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_TYPES_H
#define CUSTOM_JNA_TYPES_H
#include <cstdint>

class InstanceKlass;

/* define types */
typedef unsigned char jbool;
typedef unsigned char jbyte;
typedef unsigned short jchar;
typedef short jshort;
typedef int jint;
typedef long jlong;
typedef float jfloat;
typedef double jdouble;
typedef unsigned short fieldID;
typedef const void* object;

typedef char u1; // 1=byte
typedef uint16_t u2; // 2=byte
typedef uint32_t u3; // 3=byte
typedef uint64_t offset; // 4=byte

#endif //CUSTOM_JNA_TYPES_H