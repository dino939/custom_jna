//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_TYPES_H
#define CUSTOM_JNA_TYPES_H
#include <cstdint>
#include "jni/jni.h"

class InstanceKlass;
class ConstantPool;
class ConstantPoolCache;
class FieldInfo20;
class Method;
template <typename T>
class Array;

/* define types */
typedef unsigned short fieldID;
typedef const void* object;
typedef const intptr_t* address;

typedef uint8_t u1; // 1=byte
typedef uint16_t u2; // 2=byte
typedef uint32_t u3; // 3=byte
typedef uint64_t u4; // 4=byte
typedef uint64_t offset; // 4=byte
typedef unsigned char u_char;



const int LogBytesPerShort   = 1;
const int LogBytesPerInt     = 2;
#ifdef _LP64
const int LogBytesPerWord    = 3;
#else
const int LogBytesPerWord    = 2;
#endif
const int LogBytesPerLong    = 3;

const int BytesPerShort      = 1 << LogBytesPerShort;
const int BytesPerInt        = 1 << LogBytesPerInt;
const int BytesPerWord       = 1 << LogBytesPerWord;
const int BytesPerLong       = 1 << LogBytesPerLong;

const int LogBitsPerByte     = 3;
const int LogBitsPerShort    = LogBitsPerByte + LogBytesPerShort;
const int LogBitsPerInt      = LogBitsPerByte + LogBytesPerInt;
const int LogBitsPerWord     = LogBitsPerByte + LogBytesPerWord;
const int LogBitsPerLong     = LogBitsPerByte + LogBytesPerLong;

const int BitsPerByte        = 1 << LogBitsPerByte;
const int BitsPerShort       = 1 << LogBitsPerShort;
const int BitsPerInt         = 1 << LogBitsPerInt;
const int BitsPerWord        = 1 << LogBitsPerWord;
const int BitsPerLong        = 1 << LogBitsPerLong;

const int WordAlignmentMask  = (1 << LogBytesPerWord) - 1;
const int LongAlignmentMask  = (1 << LogBytesPerLong) - 1;

const int WordsPerLong       = 2;       // Number of stack entries for longs

const int oopSize            = sizeof(char*); // Full-width oop
extern int heapOopSize;                       // Oop within a java object
const int wordSize           = sizeof(char*);
const int longSize           = sizeof(jlong);
const int jintSize           = sizeof(jint);
const int size_tSize         = sizeof(size_t);

const int BytesPerOop        = BytesPerWord;  // Full-width oop

extern int LogBytesPerHeapOop;                // Oop within a java object
extern int LogBitsPerHeapOop;
extern int BytesPerHeapOop;
extern int BitsPerHeapOop;

const int BitsPerJavaInteger = 32;
const int BitsPerJavaLong    = 64;
const int BitsPerSize_t      = size_tSize * BitsPerByte;

const int jintAsStringSize = 12;


typedef unsigned int    juint;
typedef unsigned char   jubyte;
typedef unsigned short  jushort;


#endif //CUSTOM_JNA_TYPES_H
