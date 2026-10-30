//
// Created by prone on 04.08.2026.
//

#ifndef CUSTOM_JNA_JAVATHREAD_H
#define CUSTOM_JNA_JAVATHREAD_H

#include <cstdint>

// Minimal stand-in for the HotSpot "Thread" root type.  We only need the
// offset/distance math for resolving the current Java thread; no fields are
// accessed through this base, so it may stay empty.
class ThreadBase {
public:
    virtual ~ThreadBase() = default;
};

// Represents the HotSpot "JavaThread" oop (actually a CHeapObj in the VM,
// not an oop).  Provides a way to resolve the current thread pointer that is
// used by native functions expecting a JavaThread* (e.g. Rewriter::rewrite).
class JavaThread : public ThreadBase {
public:
    // Equivalent of HotSpot Thread::current(): returns the JavaThread* that
    // is executing on the current OS thread.  Never returns null on a running
    // Java thread inside an injected DLL; returns null otherwise.
    static JavaThread* CurrentThread();

    // HotSpot JavaThread._thread_state (JvmtiThreadState), convenience accessors.
    int  getThreadState();
    void setThreadState(int state);
};

#endif //JVM_JAVATHREAD_H