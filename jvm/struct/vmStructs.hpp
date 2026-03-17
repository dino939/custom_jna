#ifndef SHARE_RUNTIME_VMSTRUCTS_HPP
#define SHARE_RUNTIME_VMSTRUCTS_HPP
#include <cstdint>

struct VMStructEntry {
    const char* typeName;            // The type name containing the given field (example: "Klass")
    const char* fieldName;           // The field name within the type           (example: "_name")
    const char* typeString;          // Quoted name of the type of this field (example: "Symbol*";
    // parsed in Java to ensure type correctness
    int32_t  isStatic;               // Indicates whether following field is an offset or an address
    uint64_t offset;                 // Offset of field within structure; only used for nonstatic fields
    void* address;                   // Address of field; only used for static fields
    // ("offset" can not be reused because of apparent solstudio compiler bug
    // in generation of initializer data)
};

#endif