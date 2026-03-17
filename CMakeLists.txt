cmake_minimum_required(VERSION 4.0)
project(custom_jna)

set(CMAKE_CXX_STANDARD 20)

add_library(custom_jna SHARED main.cpp
        jvm/jna.h
        jvm/types.h
        jvm/struct/symbol/Symbol.cpp
        jvm/struct/symbol/Symbol.h
        jvm/utils/assert/assert.hpp
        jvm/struct/vmStructs.hpp
        jvm/offset/offsetExtractor.cpp
        jvm/offset/offsetExtractor.h
        jvm/struct/klass/instanceKlass.cpp
        jvm/struct/klass/instanceKlass.h
        jvm/struct/classloader/classLoaderData.cpp
        jvm/struct/classloader/classLoaderData.h
        jvm/jna/jnaBootStrap.cpp
        jvm/jna/jnaBootStrap.h
        jvm/jna/JvmFunctions.cpp
        jvm/jna/JvmFunctions.h
)
