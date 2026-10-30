#pragma once
#define FIELDINFO_TAG_SIZE             2
#define FIELDINFO_TAG_OFFSET           1 << 0
#define FIELDINFO_TAG_CONTENDED        1 << 1

#include "../../struct/oop/array/Array.h"

class FieldInfo_17
{
public:

    inline int build_int_from_shorts(u2 low, u2 high)
    {
        return ((int)((unsigned int)high << 16) | (unsigned int)low);
    }

    enum FieldOffset
    {
        access_flags_offset = 0,
        name_index_offset = 1,
        signature_index_offset = 2,
        initval_index_offset = 3,
        low_packed_offset = 4,
        high_packed_offset = 5,
        field_slots = 6
    };

    u2 name_index() const { return _shorts[name_index_offset]; }
    u2 signature_index() const { return _shorts[signature_index_offset]; }
    u2 access_flags() const { return _shorts[access_flags_offset]; }
    u2 initval_index() const {return _shorts[initval_index_offset];
    }
    static FieldInfo_17* from_field_array(Array<u2>* fields, int index) {
        return reinterpret_cast<FieldInfo_17 *>(fields->adr_at(index * field_slots));
    }

    u2 offset()
    {
        return build_int_from_shorts(_shorts[low_packed_offset], _shorts[high_packed_offset]) >> FIELDINFO_TAG_SIZE;
    }

private:

    u2 _shorts[field_slots];
};


class AccessFlags {
private:
public:
    int _flags;

    AccessFlags() : _flags(0) {};
    AccessFlags(int flags) : _flags(flags) {};

   enum
    {
        // See jvm.h for shared JVM_ACC_XXX access flags

        // HotSpot-specific access flags

        // flags actually put in .class file
        JVM_ACC_WRITTEN_FLAGS = 0x00007FFF,

        // Method* flags
        JVM_ACC_MONITOR_MATCH = 0x10000000,     // True if we know that monitorenter/monitorexit bytecodes match
        JVM_ACC_HAS_MONITOR_BYTECODES = 0x20000000,     // Method contains monitorenter/monitorexit bytecodes
        JVM_ACC_HAS_LOOPS = 0x40000000,     // Method has loops
        JVM_ACC_LOOPS_FLAG_INIT = (int)0x80000000,// The loop flag has been initialized
        JVM_ACC_QUEUED = 0x01000000,     // Queued for compilation
        JVM_ACC_NOT_C2_COMPILABLE = 0x02000000,
        JVM_ACC_NOT_C1_COMPILABLE = 0x04000000,
        JVM_ACC_NOT_C2_OSR_COMPILABLE = 0x08000000,
        JVM_ACC_HAS_LINE_NUMBER_TABLE = 0x00100000,
        JVM_ACC_HAS_CHECKED_EXCEPTIONS = 0x00400000,
        JVM_ACC_HAS_JSRS = 0x00800000,
        JVM_ACC_IS_OLD = 0x00010000,     // RedefineClasses() has replaced this method
        JVM_ACC_IS_OBSOLETE = 0x00020000,     // RedefineClasses() has made method obsolete
        JVM_ACC_IS_PREFIXED_NATIVE = 0x00040000,     // JVMTI has prefixed this native method
        JVM_ACC_ON_STACK = 0x00080000,     // RedefineClasses() was used on the stack
        JVM_ACC_IS_DELETED = 0x00008000,     // RedefineClasses() has deleted this method

        // Klass* flags
        JVM_ACC_HAS_MIRANDA_METHODS = 0x10000000,     // True if this class has miranda methods in it's vtable
        JVM_ACC_HAS_VANILLA_CONSTRUCTOR = 0x20000000,     // True if klass has a vanilla default constructor
        JVM_ACC_HAS_FINALIZER = 0x40000000,     // True if klass has a non-empty finalize() method
        JVM_ACC_IS_CLONEABLE_FAST = (int)0x80000000,// True if klass implements the Cloneable interface and can be optimized in generated code
        JVM_ACC_HAS_FINAL_METHOD = 0x01000000,     // True if klass has final method
        JVM_ACC_IS_SHARED_CLASS = 0x02000000,     // True if klass is shared
        JVM_ACC_IS_HIDDEN_CLASS = 0x04000000,     // True if klass is hidden
        JVM_ACC_IS_VALUE_BASED_CLASS = 0x08000000,     // True if klass is marked as a ValueBased class

        // Klass* and Method* flags
        JVM_ACC_HAS_LOCAL_VARIABLE_TABLE = 0x00200000,

        JVM_ACC_PROMOTED_FLAGS = 0x00200000,     // flags promoted from methods to the holding klass

        // field flags
        // Note: these flags must be defined in the low order 16 bits because
        // InstanceKlass only stores a ushort worth of information from the
        // AccessFlags value.
        // These bits must not conflict with any other field-related access flags
        // (e.g., ACC_ENUM).
        // Note that the class-related ACC_ANNOTATION bit conflicts with these flags.
        JVM_ACC_FIELD_ACCESS_WATCHED = 0x00002000, // field access is watched by JVMTI
        JVM_ACC_FIELD_MODIFICATION_WATCHED = 0x00008000, // field modification is watched by JVMTI
        JVM_ACC_FIELD_INTERNAL = 0x00000400, // internal field, same as JVM_ACC_ABSTRACT
        JVM_ACC_FIELD_STABLE = 0x00000020, // @Stable field, same as JVM_ACC_SYNCHRONIZED and JVM_ACC_SUPER
        JVM_ACC_FIELD_INITIALIZED_FINAL_UPDATE = 0x00000100, // (static) final field updated outside (class) initializer, same as JVM_ACC_NATIVE
        JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE = 0x00000800, // field has generic signature

        JVM_ACC_FIELD_INTERNAL_FLAGS = JVM_ACC_FIELD_ACCESS_WATCHED |
        JVM_ACC_FIELD_MODIFICATION_WATCHED |
        JVM_ACC_FIELD_INTERNAL |
        JVM_ACC_FIELD_STABLE |
        JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE,

        JVM_ACC_PUBLIC       = 0x0001,  /* visible to everyone */
        JVM_ACC_PRIVATE      = 0x0002,  /* visible only to the defining class */
        JVM_ACC_PROTECTED    = 0x0004,  /* visible to subclasses */
        JVM_ACC_STATIC       = 0x0008,  /* instance variable is static */
        JVM_ACC_FINAL        = 0x0010,  /* no further subclassing, overriding */
        JVM_ACC_SYNCHRONIZED = 0x0020,  /* wrap method call in monitor lock */
        JVM_ACC_SUPER        = 0x0020,  /* funky handling of invokespecial */
        JVM_ACC_VOLATILE     = 0x0040,  /* can not cache in registers */
        JVM_ACC_BRIDGE       = 0x0040,  /* bridge method generated by compiler */
        JVM_ACC_TRANSIENT    = 0x0080,  /* not persistent */
        JVM_ACC_VARARGS      = 0x0080,  /* method declared with variable number of args */
        JVM_ACC_NATIVE       = 0x0100,  /* implemented in C */
        JVM_ACC_INTERFACE    = 0x0200,  /* class is an interface */
        JVM_ACC_ABSTRACT     = 0x0400,  /* no definition provided */
        JVM_ACC_STRICT       = 0x0800,  /* strict floating point */
        JVM_ACC_SYNTHETIC    = 0x1000,  /* compiler-generated class, method or field */
        JVM_ACC_ANNOTATION   = 0x2000,  /* annotation type */
        JVM_ACC_ENUM         = 0x4000  /* field is declared as element of enum */
    };

#define JVM_RECOGNIZED_FIELD_MODIFIERS (JVM_ACC_PUBLIC | \
JVM_ACC_PRIVATE | \
JVM_ACC_PROTECTED | \
JVM_ACC_STATIC | \
JVM_ACC_FINAL | \
JVM_ACC_VOLATILE | \
JVM_ACC_TRANSIENT | \
JVM_ACC_ENUM | \
JVM_ACC_SYNTHETIC)

    // Java access flags
    bool is_public      () const         { return (_flags & JVM_ACC_PUBLIC      ) != 0; }
    bool is_private     () const         { return (_flags & JVM_ACC_PRIVATE     ) != 0; }
    bool is_protected   () const         { return (_flags & JVM_ACC_PROTECTED   ) != 0; }
    bool is_static      () const         { return (_flags & JVM_ACC_STATIC      ) != 0; }
    bool is_final       () const         { return (_flags & JVM_ACC_FINAL       ) != 0; }
    bool is_synchronized() const         { return (_flags & JVM_ACC_SYNCHRONIZED) != 0; }
    bool is_super       () const         { return (_flags & JVM_ACC_SUPER       ) != 0; }
    bool is_volatile    () const         { return (_flags & JVM_ACC_VOLATILE    ) != 0; }
    bool is_transient   () const         { return (_flags & JVM_ACC_TRANSIENT   ) != 0; }
    bool is_native      () const         { return (_flags & JVM_ACC_NATIVE      ) != 0; }
    bool is_interface   () const         { return (_flags & JVM_ACC_INTERFACE   ) != 0; }
    bool is_abstract    () const         { return (_flags & JVM_ACC_ABSTRACT    ) != 0; }

    // Attribute flags
    bool is_synthetic   () const         { return (_flags & JVM_ACC_SYNTHETIC   ) != 0; }

    // Klass* flags
    bool has_finalizer           () const { return (_flags & JVM_ACC_HAS_FINALIZER          ) != 0; }
    bool is_cloneable_fast       () const { return (_flags & JVM_ACC_IS_CLONEABLE_FAST      ) != 0; }
    bool is_hidden_class         () const { return (_flags & JVM_ACC_IS_HIDDEN_CLASS        ) != 0; }
    bool is_value_based_class    () const { return (_flags & JVM_ACC_IS_VALUE_BASED_CLASS   ) != 0; }


    int get_flags               () const { return (_flags & JVM_ACC_WRITTEN_FLAGS); }

    void set_field_flags(int flags)      {
        _flags = (flags & JVM_RECOGNIZED_FIELD_MODIFIERS);
    }
    void set_flags(int flags)            { _flags = (flags & JVM_ACC_WRITTEN_FLAGS); }
    bool field_has_generic_signature() const
    { return (_flags & JVM_ACC_FIELD_HAS_GENERIC_SIGNATURE) != 0; }
};

static constexpr u4 flag_mask(int pos) {
    return (u4)1 << pos;
};

class FieldFlags {
public:
    enum FieldFlagBitPosition {
        _ff_initialized,  // has ConstantValue initializer attribute
        _ff_injected,     // internal field injected by the JVM
        _ff_generic,      // has a generic signature
        _ff_stable,       // trust as stable b/c declared as @Stable
        _ff_contended,    // is contended, may have contention-group
      };
    u4 _flags;

    FieldFlags(u4 flags) {
        _flags = flags;
    }

    static const u4 _optional_item_bit_mask =
    flag_mask((int)_ff_initialized) |
    flag_mask((int)_ff_generic)     |
    flag_mask((int)_ff_contended);

    bool test_flag(FieldFlagBitPosition pos) const {
        return (_flags & flag_mask(pos)) != 0;
    }

    void update_flag(FieldFlagBitPosition pos, bool z) {
        if (z)    _flags |=  flag_mask(pos);
        else      _flags &= ~flag_mask(pos);
    }

    bool is_initialized() const     { return test_flag(_ff_initialized); }
    bool is_injected() const        { return test_flag(_ff_injected); }
    bool is_generic() const         { return test_flag(_ff_generic); }
    bool is_stable() const          { return test_flag(_ff_stable); }
    bool is_contended() const       { return test_flag(_ff_contended); }

    void update_initialized(bool z) { update_flag(_ff_initialized, z); }
    void update_injected(bool z)    { update_flag(_ff_injected, z); }
    void update_generic(bool z)     { update_flag(_ff_generic, z); }
    void update_stable(bool z)      { update_flag(_ff_stable, z); }
    void update_contended(bool z)   { update_flag(_ff_contended, z); }

};

class FieldInfo_20 {
    public:
    u4 _index;                    // which field it is
    u2 _name_index;               // index in CP of name
    u2 _signature_index;          // index in CP of descriptor
    u4 _offset;                   // offset in object layout
    AccessFlags _access_flags;    // access flags (JVM spec)
    FieldFlags _field_flags;      // VM defined flags (not JVM spec)
    u2 _initializer_index;        // index from ConstantValue attr (or 0)
    u2 _generic_signature_index;  // index from GenericSignature attr (or 0)
    u2 _contention_group;         // index from @Contended group item (or 0)

    FieldInfo_20() :
           _index(0),
           _name_index(0),
           _signature_index(0),
           _offset(0),
           _access_flags(AccessFlags(0)),
           _field_flags(FieldFlags(0)),
           _initializer_index(0),
           _generic_signature_index(0),
           _contention_group(0) {}


    u2 name_index() const                      { return _name_index; }
    u2 signature_index() const                 { return _signature_index; }
    u4 offset() const                          { return _offset; }
    AccessFlags access_flags() const           { return _access_flags; }
    FieldFlags field_flags() const             { return _field_flags; }
    FieldFlags* field_flags_addr()             { return &_field_flags; }
    u2 initializer_index() const               { return _initializer_index; }
    u2 generic_signature_index() const         { return _generic_signature_index; }
    u2 contention_group() const                { return _contention_group; }
};