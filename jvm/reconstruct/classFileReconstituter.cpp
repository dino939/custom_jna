#include "classFileReconstituter.h"

#include "Windows.h"
#include <cstdio>
#include <fstream>
#include <vector>
#include "../../jvm/struct/oop/constantPool/ConstantPool.h"
#include "../../jvm/struct/oop/constantPool/ConstantPoolCache.h"
#include "../../jvm/struct/oop/klass/InnerClassesIterator.h"
#include "../../jvm/struct/oop/klass/RecordComponent.h"
#include "../../jvm/struct/oop/method/exceptionTable.h"
#include "../../jvm/struct/oop/method/writableStream/CompressedLineNumberReadStream.h"
#include "../../utils/bytecodeRewriter.h"


inline int extract_low_short_from_int(long x) {
    return x & 0xffff;
}

inline int extract_high_short_from_int(long x) {
    return (x >> 16) & 0xffff;
}



ClassFileReconstituter::ClassFileReconstituter(InstanceKlass *ik) {
    this->_ik = ik;
    std::string className = ik->name()->as_string();
    int res = className.find_last_of('/');
    if (res != -1) {
        className = className.substr(res + 1);
    }

    std::string temp = (std::filesystem::temp_directory_path() / (className + ".class")).string();

    this->stream.open(temp, std::ios::binary);
    if (!this->stream.is_open()) {
        return;
    }

    write_u4(0xCAFEBABE);
    write_u2(ik->constants()->minor_version());
    write_u2(ik->constants()->major_version());

    //constant pool
    {
        write_u2(ik->constants()->length());
        for (long idx = 1; idx < ik->constants()->length(); idx++) {
            auto tag = static_cast<ConstantType>(ik->constants()->tags()->at(idx));

            switch (tag) {
                case JVM_CONSTANT_Invalid: {
                    break;
                }
                case JVM_CONSTANT_Unicode: {
                    break;
                }
                case JVM_CONSTANT_Utf8: {
                    write_u1(JVM_CONSTANT_Utf8);
                    Symbol *sym = ik->constants()->symbol_at(idx);
                    write_utf8(sym);
                    break;
                }
                case JVM_CONSTANT_Integer: {
                    write_u1(JVM_CONSTANT_Integer);
                    jint val = *ik->constants()->int_at_addr(idx);
                    write_u4(val);
                    break;
                }
                case JVM_CONSTANT_Float: {
                    write_u1(JVM_CONSTANT_Float);
                    jfloat val = *ik->constants()->float_at_addr(idx);
                    uint32_t bits;
                    memcpy(&bits, &val, sizeof(bits));
                    write_u4(bits);
                    break;
                }
                case JVM_CONSTANT_Long: {
                    write_u1(JVM_CONSTANT_Long);
                    jlong val = *ik->constants()->long_at_addr(idx);
                    write_u8(val);
                    idx++;
                    break;
                }
                case JVM_CONSTANT_Double: {
                    write_u1(JVM_CONSTANT_Double);
                    jdouble val = *ik->constants()->double_at_addr(idx);
                    uint64_t bits;
                    memcpy(&bits, &val, sizeof(bits));
                    write_u8(bits);
                    idx++;
                    break;
                }
                case JVM_CONSTANT_Class:
                case JVM_CONSTANT_UnresolvedClass:
                case JVM_CONSTANT_UnresolvedClassInError: {
                    write_u1(JVM_CONSTANT_Class);
                    write_u2(static_cast<u2>(ik->constants()->klass_slot_at(idx)));
                    break;
                }
                case JVM_CONSTANT_String: {
                    write_u1(JVM_CONSTANT_String);
                    // string_index обязан указывать на CONSTANT_Utf8.
                    Symbol *sym = ik->constants()->symbol_at(idx);
                    write_u2(symbolToConstantPoolIndex(sym->as_string()));
                    break;
                }
                case JVM_CONSTANT_Fieldref:
                case JVM_CONSTANT_Methodref:
                case JVM_CONSTANT_InterfaceMethodref: {
                    write_u1(static_cast<uint8_t>(tag));
                    write_u2(ik->constants()->uncached_klass_ref_index_at(idx));
                    write_u2(ik->constants()->uncached_name_and_type_ref_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_NameAndType: {
                    write_u1(JVM_CONSTANT_NameAndType);
                    write_u2(ik->constants()->name_ref_index_at(idx));
                    write_u2(ik->constants()->signature_ref_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_ClassIndex: {
                    write_u1(JVM_CONSTANT_Class);
                    write_u2(ik->constants()->klass_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_StringIndex: {
                    write_u1(JVM_CONSTANT_String);
                    write_u2(ik->constants()->string_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_MethodHandle:
                case JVM_CONSTANT_MethodHandleInError: {
                    write_u1(JVM_CONSTANT_MethodHandle);
                    write_u1(ik->constants()->method_handle_ref_kind_at(idx));
                    write_u2(ik->constants()->method_handle_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_MethodType:
                case JVM_CONSTANT_MethodTypeInError: {
                    write_u1(JVM_CONSTANT_MethodType);
                    write_u2(ik->constants()->method_type_index_at(idx));
                    break;
                }
                case JVM_CONSTANT_Dynamic:
                case JVM_CONSTANT_DynamicInError: {
                    write_u1(JVM_CONSTANT_Dynamic);
                    write_u2(extract_low_short_from_int(*ik->constants()->int_at_addr(idx)));
                    write_u2(extract_high_short_from_int(*ik->constants()->int_at_addr(idx)));
                    break;
                }
                case JVM_CONSTANT_InvokeDynamic: {
                    write_u1(JVM_CONSTANT_InvokeDynamic);
                    write_u2(extract_low_short_from_int(*ik->constants()->int_at_addr(idx)));
                    write_u2(extract_high_short_from_int(*ik->constants()->int_at_addr(idx)));
                    break;
                }
                default:
                    break;
            }
        }
    }

    write_u2(ik->access_flags().get_flags() & JVM_RECOGNIZED_CLASS_MODIFIERS);
    write_u2(classNameToConstantPoolIndex(ik->name()->as_string())); //this_class_index
    write_u2(ik->super() == NULL ? 0 : classNameToConstantPoolIndex(ik->super()->name()->as_string()));
    //super_class_index

    //interfaces
    {
        Array<InstanceKlass *> *interfaces = ik->local_interfaces();
        int num_interfaces = interfaces->length();
        write_u2(num_interfaces);
        for (int index = 0; index < num_interfaces; index++) {
            InstanceKlass *iik = interfaces->at(index);
            write_u2(classNameToConstantPoolIndex(iik->name()->as_string()));
        }
    }

    //fields
    if (ik->fieldinfo_stream() != nullptr) {
        // ===== JAVA 21+ =====
        Array<AnnotationArray *> *fields_anno = nullptr;
        Array<AnnotationArray *> *fields_type_anno = nullptr;
        if (ik->annotations() != nullptr) {
            fields_anno = ik->annotations()->fields_annotations();
            fields_type_anno = ik->annotations()->fields_type_annotations();
        }

        FieldInfoReader field_reader(ik->fieldinfo_stream());

        int num_java = field_reader.next_unit();
        int num_injected = field_reader.next_unit();
        int total_fields = num_java + num_injected;

        write_u2(num_java);

        FieldInfo_20 field;
        int java_field_index = 0;

        for (long i = 0; i < total_fields; i++) {
            field_reader.read_field_info(field);

            if (field.field_flags().is_injected()) {
                continue;
            }

            AccessFlags access_flags = field.access_flags();
            int name_index = field.name_index();
            int signature_index = field.signature_index();
            int initial_value_index = field.initializer_index();
            int generic_signature_index = field.generic_signature_index();

            AnnotationArray *anno = (fields_anno == nullptr) ? nullptr : fields_anno->at(java_field_index);
            AnnotationArray *type_anno = (fields_type_anno == nullptr)
                                             ? nullptr
                                             : fields_type_anno->at(java_field_index);
            java_field_index++;

            write_u2(access_flags.get_flags() & JVM_RECOGNIZED_FIELD_MODIFIERS);
            write_u2(name_index);
            write_u2(signature_index);

            int attr_count = 0;
            if (initial_value_index != 0) attr_count++;
            if (generic_signature_index != 0) attr_count++;
            if (anno != nullptr) attr_count++;
            if (type_anno != nullptr) attr_count++;

            write_u2(attr_count);

            if (initial_value_index != 0) {
                write_attribute_name_index("ConstantValue");
                write_u4(2);
                write_u2(initial_value_index);
            }
            if (generic_signature_index != 0) {
                write_attribute_name_index("Signature");
                write_u4(2);
                write_u2(generic_signature_index);
            }
            if (anno != nullptr) {
                unsigned int length = anno->length();
                write_attribute_name_index("RuntimeVisibleAnnotations");
                write_u4(length);
                stream.write(reinterpret_cast<const char *>(anno->adr_at(0)), length);
            }
            if (type_anno != nullptr) {
                unsigned int length = type_anno->length();
                write_attribute_name_index("RuntimeVisibleTypeAnnotations");
                write_u4(length);
                stream.write(reinterpret_cast<const char *>(type_anno->adr_at(0)), length);
            }
        }
    } else if (ik->fields() != nullptr && ik->fields()->length() > 0) {
        Array<AnnotationArray *> *fields_anno = nullptr;
        Array<AnnotationArray *> *fields_type_anno = nullptr;
        if (ik->annotations() != nullptr) {
            fields_anno = ik->annotations()->fields_annotations();
            fields_type_anno = ik->annotations()->fields_type_annotations();
        }
        int java_fields = ik->java_fields_count();

        write_u2(java_fields);

        for (long i = 0; i < java_fields; i++) {
            FieldInfo_17 *field = FieldInfo_17::from_field_array(ik->fields(), i);
            AccessFlags access_flags;
            access_flags.set_flags(field->access_flags());
            int name_index = field->name_index();
            int signature_index = field->signature_index();
            int initial_value_index = field->initval_index();

            int _generic_signature_slot = 0;
            //generic signature
            {
                int length = ik->fields()->length();
                int skipped_generic_signature_slots = 0;
                FieldInfo_17 *fi;
                AccessFlags flags;

                for (int i = 0; i * 6 < length; i++) {
                    fi = FieldInfo_17::from_field_array(ik->fields(), i);
                    flags.set_flags(fi->access_flags());
                    if (flags.field_has_generic_signature()) {
                        length--;
                    }
                }
                _generic_signature_slot = length + skipped_generic_signature_slots;
            }

            int generic_signature_index = 0;

            if (access_flags.field_has_generic_signature()) {
                generic_signature_index = ik->fields()->at(_generic_signature_slot);
            }

            AnnotationArray *anno = fields_anno == NULL ? NULL : fields_anno->at(i);
            AnnotationArray *type_anno = fields_type_anno == NULL ? NULL : fields_type_anno->at(i);

            write_u2(access_flags.get_flags() & JVM_RECOGNIZED_FIELD_MODIFIERS);
            write_u2(name_index);
            write_u2(signature_index);

            int attr_count = 0;
            if (initial_value_index != 0)
                ++attr_count;
            if (access_flags.field_has_generic_signature())
                ++attr_count;
            if (anno != nullptr)
                ++attr_count;
            if (type_anno != nullptr)
                ++attr_count;

            write_u2(attr_count);

            if (initial_value_index != 0) {
                write_attribute_name_index("ConstantValue");
                write_u4(2);
                write_u2(initial_value_index);
            }
            if (generic_signature_index != 0) {
                write_attribute_name_index("Signature");
                write_u4(2);
                write_u2(generic_signature_index);
            }
            if (anno != nullptr) {
                unsigned int length = anno->length();
                write_attribute_name_index("RuntimeVisibleAnnotations");
                write_u4(length);
                stream.write(reinterpret_cast<const char *>(anno->adr_at(0)), length);
            }
            if (type_anno != nullptr) {
                unsigned int length = type_anno->length();
                write_attribute_name_index("RuntimeVisibleTypeAnnotations");
                write_u4(length);
                stream.write(reinterpret_cast<const char *>(type_anno->adr_at(0)), length);
            }
        }
    } else {
        write_u2(0);
    }
    // methods info — пока 0 (слой Method/ConstMethod ещё не реализует name/signature/Code)
    // //methods info

    {
        Array<Method *> *methods = ik->methods();
        int num_methods = methods->length();
        int num_overpass = 0;

        for (int index = 0; index < num_methods; index++) {
            Method *method = methods->at(index);
            if (method->is_overpass()) {
                num_overpass++;
            }
        }

        write_u2(num_methods - num_overpass);

        Array<int> *method_ordering = ik->method_ordering();
        if (method_ordering != nullptr && method_ordering->length() == num_methods) {
            // _method_ordering[sorted_index] = original_index — восстанавливаем
            // исходный порядок объявления методов.
            std::unordered_map<int, int> method_order;
            for (int index = 0; index < num_methods; index++) {
                int original_index = method_ordering->at(index);
                method_order[original_index] = index;
            }

            for (int original_index = 0; original_index < num_methods; original_index++) {
                write_method_info(methods->at(method_order[original_index]));
            }
        } else {
            // _method_ordering отсутствует (Universe::the_empty_int_array,
            // length 0) — пишем методы в текущем порядке массива.
            for (int index = 0; index < num_methods; index++) {
                write_method_info(methods->at(index));
            }
        }
    }

    // class attributes
    {
        ConstantPool *cp = ik->constants();

        // BootstrapMethods is required if the pool has InvokeDynamic/Dynamic entries.
        // HotSpot _operands (Array<u2>): first the offset table (2 slots per bootstrap),
        // then the data [bsm_ref, argc, args...].
        Array<u2> *operands = cp->operands();
        int num_bootstrap_methods = 0;
        if (operands != nullptr && operands->length() > 0)
            num_bootstrap_methods = operands->at(0) / 2;

        u2 source_file_index = cp->source_file_name_index();
        u2 class_signature_index = cp->generic_signature_index();

        AnnotationArray *class_anno = nullptr;
        AnnotationArray *class_type_anno = nullptr;
        if (ik->annotations() != nullptr) {
            class_anno = ik->annotations()->class_annotations();
            class_type_anno = ik->annotations()->class_type_annotations();
        }

        const char *source_debug_extension = ik->source_debug_extension();
        bool has_source_debug_extension = source_debug_extension != nullptr && source_debug_extension[0] != '\0';

        bool has_inner_classes = false;
        bool has_enclosing_method = false;
        if (ik->inner_classes() != nullptr && ik->inner_classes()->length() > 0) {
            int ic_len = ik->inner_classes()->length();
            // An EnclosingMethod attribute exists only when the trailing
            // enclosing-method class index is actually set. A plain class that
            // merely references inner classes has spurious trailing zeros and
            // must not emit the attribute.
            has_enclosing_method = ik->enclosing_method_class_index() != 0;
            has_inner_classes = (ic_len >= InstanceKlass::inner_class_next_offset) &&
                                (!has_enclosing_method || ic_len > InstanceKlass::enclosing_method_attribute_size);
        }

        bool has_nest_host = ik->nest_host_index() != 0;
        bool has_nest_members = ik->nest_members() != nullptr && ik->nest_members()->length() > 0;
        bool has_permitted_subclasses = ik->permitted_subclasses() != nullptr &&
                                        ik->permitted_subclasses()->length() > 0;
        bool has_record = ik->record_components() != nullptr && ik->record_components()->length() > 0;

        int attr_count = 0;
        if (class_signature_index != 0) attr_count++;
        if (source_file_index != 0) attr_count++;
        if (class_anno != nullptr && class_anno->length() > 0) attr_count++;
        if (class_type_anno != nullptr && class_type_anno->length() > 0) attr_count++;
        if (num_bootstrap_methods > 0) attr_count++;
        if (has_source_debug_extension) attr_count++;
        if (has_inner_classes) attr_count++;
        if (has_enclosing_method) attr_count++;
        if (has_nest_host) attr_count++;
        if (has_nest_members) attr_count++;
        if (has_permitted_subclasses) attr_count++;
        if (has_record) attr_count++;

        write_u2(attr_count);

        if (class_signature_index != 0) {
            write_attribute_name_index("Signature");
            write_u4(2);
            write_u2(class_signature_index);
        }
        if (source_file_index != 0) {
            write_attribute_name_index("SourceFile");
            write_u4(2);
            write_u2(source_file_index);
        }
        if (class_anno != nullptr && class_anno->length() > 0) {
            unsigned int length = class_anno->length();
            write_attribute_name_index("RuntimeVisibleAnnotations");
            write_u4(length);
            stream.write(reinterpret_cast<const char *>(class_anno->adr_at(0)), length);
        }
        if (class_type_anno != nullptr && class_type_anno->length() > 0) {
            unsigned int length = class_type_anno->length();
            write_attribute_name_index("RuntimeVisibleTypeAnnotations");
            write_u4(length);
            stream.write(reinterpret_cast<const char *>(class_type_anno->adr_at(0)), length);
        }
        if (has_source_debug_extension) {
            write_source_debug_extension_attribute(ik);
        }
        if (has_inner_classes) {
            write_inner_classes_attribute(ik);
        }
        if (has_enclosing_method) {
            write_enclosing_method_attribute(ik);
        }
        if (has_nest_host) {
            write_nest_host_attribute(ik);
        }
        if (has_nest_members) {
            write_nest_members_attribute(ik);
        }
        if (has_permitted_subclasses) {
            write_permitted_subclasses_attribute(ik);
        }
        if (has_record) {
            write_record_attribute(ik);
        }
        if (num_bootstrap_methods > 0) {
            int length = 2; // num_bootstrap_methods (u2)
            for (int n = 0; n < num_bootstrap_methods; n++) {
                int offset = operands->at(n * 2);
                int argc = operands->at(offset + 1);
                length += 2 + 2 + 2 * argc; // bsm_ref + argc + args
            }
            write_attribute_name_index("BootstrapMethods");
            write_u4(length);
            write_u2(num_bootstrap_methods);
            for (int n = 0; n < num_bootstrap_methods; n++) {
                int offset = operands->at(n * 2);
                int bsm_ref = operands->at(offset + 0);
                int argc = operands->at(offset + 1);
                write_u2(bsm_ref);
                write_u2(argc);
                for (int a = 0; a < argc; a++)
                    write_u2(operands->at(offset + 2 + a));
            }
        }
    }

    //MessageBoxA(NULL, "9", "jvmclassdumper", MB_ICONERROR);

    // //methods info
    // {
    //     Array<Method*>* methods = ik->methods();
    //     int num_methods = methods->length();
    //     int num_overpass = 0;
    //
    //     for (int index = 0; index < num_methods; index++)
    //     {
    //         Method* method = methods->at(index);
    //         if (method->is_overpass())
    //         {
    //             num_overpass++;
    //         }
    //     }
    //
    //     write_u2(num_methods - num_overpass);
    //     int index;
    //     int original_index;
    //     std::unordered_map<int, int> method_order;
    //
    //     for (index = 0; index < num_methods; index++)
    //     {
    //         original_index = ik->method_ordering()->at(index);
    //         method_order[original_index] = index;
    //     }
    //
    //     for (original_index = 0; original_index < num_methods; original_index++)
    //     {
    //         index = method_order[original_index];
    //         write_method_info(methods->at(index));
    //     }
    //     /*for (int index = 0; index < num_methods; index++) {
    //         write_method_info(methods->get(index));
    //     }*/
    // }

    //MessageBoxA(NULL, "10", "jvmclassdumper", MB_ICONERROR);

    // //class attributes
    // {
    //     uint16_t inner_classes_length = inner_classes_attribute_length(ik);
    //     Symbol* generic_signature = ik->constants()->generic_signature();
    //     AnnotationArray* anno = ik->class_annotations();
    //     AnnotationArray* type_anno = ik->class_type_annotations();
    //
    //     int attr_count = 0;
    //
    //     if (generic_signature != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->constants()->source_file_name() != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->source_debug_extension() != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (inner_classes_length > 0)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (anno != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (type_anno != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->constants()->_operands != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->_nest_host_index != 0)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->_nest_members != nullptr)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->_permitted_subclasses != nullptr)
    //     {
    //         ++attr_count;
    //     }
    //
    //     if (ik->_record_components != NULL)
    //     {
    //         ++attr_count;
    //     }
    //
    //     write_u2(attr_count);
    //
    //     if (generic_signature != NULL)
    //     {
    //         write_attribute_name_index("Signature");
    //         write_u4(2);
    //         write_u2(symbolToConstantPoolIndex(generic_signature->as_string()));
    //     }
    //
    //     if (ik->constants()->source_file_name() != NULL)
    //     {
    //         write_attribute_name_index("SourceFile");
    //         write_u4(2);
    //         write_u2(symbolToConstantPoolIndex(ik->constants()->source_file_name()->as_string()));
    //     }
    //
    //     if (ik->_source_debug_extension != NULL)
    //     {
    //         write_attribute_name_index("SourceDebugExtension");
    //         int len = (int)strlen(ik->_source_debug_extension);
    //         write_u4(len);
    //         uint8_t* ext = (uint8_t*)ik->_source_debug_extension;
    //         for (int i = 0; i < len; i++)
    //         {
    //             write_u1(ext[i]);
    //         }
    //     }
    //
    //     if (anno != NULL)
    //     {
    //         unsigned int length = anno->length();
    //         write_attribute_name_index("RuntimeVisibleAnnotations");
    //         write_u4(length);
    //         stream.write(reinterpret_cast<const char*>(anno->adr_at(0)), length);
    //     }
    //
    //     if (type_anno != NULL)
    //     {
    //         unsigned int length = type_anno->length();
    //         write_attribute_name_index("RuntimeVisibleTypeAnnotations");
    //         write_u4(length);
    //         stream.write(reinterpret_cast<const char*>(type_anno->adr_at(0)), length);
    //     }
    //
    //     if (ik->_nest_host_index != 0)
    //     {
    //         int length = sizeof(uint16_t);
    //         int host_class_index = ik->_nest_host_index;
    //
    //         write_attribute_name_index("NestHost");
    //         write_u4(length);
    //         write_u2(host_class_index);
    //     }
    //
    //     if (ik->_nest_members != nullptr)
    //     {
    //         Array<uint16_t>* nest_members = ik->_nest_members;
    //         int number_of_classes = nest_members->length();
    //         int length = sizeof(uint16_t) * (1 + number_of_classes);
    //
    //         write_attribute_name_index("NestMembers");
    //         write_u4(length);
    //         write_u2(number_of_classes);
    //         for (int i = 0; i < number_of_classes; i++)
    //         {
    //             uint16_t class_cp_index = nest_members->at(i);
    //             write_u2(class_cp_index);
    //         }
    //     }
    //
    //     if (ik->_permitted_subclasses != nullptr)
    //     {
    //         Array<uint16_t>* permitted_subclasses = ik->_permitted_subclasses;
    //         int number_of_classes = permitted_subclasses->length();
    //         int length = sizeof(uint16_t) * (1 + number_of_classes);
    //
    //         write_attribute_name_index("PermittedSubclasses");
    //         write_u4(length);
    //         write_u2(number_of_classes);
    //         for (int i = 0; i < number_of_classes; i++)
    //         {
    //             uint16_t class_cp_index = permitted_subclasses->at(i);
    //             write_u2(class_cp_index);
    //         }
    //     }
    //
    //     if (ik->_record_components != NULL)
    //     {
    //         Array<RecordComponent*>* components = ik->_record_components;
    //         int number_of_components = components->length();
    //
    //         int length = sizeof(uint16_t) + (sizeof(uint16_t) * 3 * number_of_components);
    //         for (int x = 0; x < number_of_components; x++)
    //         {
    //             RecordComponent* component = components->at(x);
    //             if (component->_generic_signature_index != 0)
    //             {
    //                 length += 8;
    //             }
    //             if (component->_annotations != NULL)
    //             {
    //                 length += 6 + component->_annotations->length();
    //             }
    //             if (component->_type_annotations != NULL)
    //             {
    //                 length += 6 + component->_type_annotations->length();
    //             }
    //         }
    //
    //         write_attribute_name_index("Record");
    //         write_u4(length);
    //         write_u2(number_of_components);
    //         for (int i = 0; i < number_of_components; i++)
    //         {
    //             RecordComponent* component = components->at(i);
    //             write_u2(component->_name_index);
    //             write_u2(component->_descriptor_index);
    //             write_u2(component->_attributes_count);
    //             if (component->_generic_signature_index != 0)
    //             {
    //                 write_attribute_name_index("Signature");
    //                 write_u4(2);
    //                 write_u2(component->_generic_signature_index);
    //             }
    //             if (component->_annotations != NULL)
    //             {
    //                 uint32_t length = component->_annotations->length();
    //                 write_attribute_name_index("RuntimeVisibleAnnotations");
    //                 write_u4(length);
    //                 stream.write(reinterpret_cast<const char*>(component->_annotations->adr_at(0)), length);
    //             }
    //             if (component->_type_annotations != NULL)
    //             {
    //                 uint32_t length = component->_type_annotations->length();
    //                 write_attribute_name_index("RuntimeVisibleTypeAnnotations");
    //                 write_u4(length);
    //                 stream.write(reinterpret_cast<const char*>(component->_type_annotations->adr_at(0)), length);
    //             }
    //         }
    //     }
    //
    //     if (ik->constants()->_operands != NULL)
    //     {
    //         Array<uint16_t>* operands = ik->constants()->_operands;
    //         write_attribute_name_index("BootstrapMethods");
    //         int num_bootstrap_methods = ik->constants()->operand_array_length(operands);
    //
    //         int length = sizeof(uint16_t);
    //         for (int n = 0; n < num_bootstrap_methods; n++)
    //         {
    //             uint16_t num_bootstrap_arguments = ik->constants()->operand_argument_count_at(n);
    //             length += sizeof(uint16_t);
    //             length += sizeof(uint16_t);
    //             length += sizeof(uint16_t) * num_bootstrap_arguments;
    //         }
    //         write_u4(length);
    //
    //         write_u2(num_bootstrap_methods);
    //         for (int n = 0; n < num_bootstrap_methods; n++)
    //         {
    //             uint16_t bootstrap_method_ref = ik->constants()->operand_bootstrap_method_ref_index_at(n);
    //             uint16_t num_bootstrap_arguments = ik->constants()->operand_argument_count_at(n);
    //             write_u2(bootstrap_method_ref);
    //             write_u2(num_bootstrap_arguments);
    //             for (int arg = 0; arg < num_bootstrap_arguments; arg++)
    //             {
    //                 uint16_t bootstrap_argument = ik->constants()->operand_argument_index_at(n, arg);
    //                 write_u2(bootstrap_argument);
    //             }
    //         }
    //     }
    //
    //     if (inner_classes_length > 0)
    //     {
    //         InnerClassesIterator iter(ik);
    //         uint16_t entry_count = inner_classes_length / InstanceKlass::inner_class_next_offset;
    //         uint32_t size = 2 + entry_count * (2 + 2 + 2 + 2);
    //
    //         write_attribute_name_index("InnerClasses");
    //         write_u4(size);
    //         write_u2(entry_count);
    //         for (; !iter.done(); iter.next())
    //         {
    //             write_u2(iter.inner_class_info_index());
    //             write_u2(iter.outer_class_info_index());
    //             write_u2(iter.inner_name_index());
    //             write_u2(iter.inner_access_flags());
    //         }
    //     }
    // }

    this->stream.close();

    // Read the freshly-written class file bytes back into a byte vector.
    {
        std::ifstream in(temp, std::ios::binary);
        if (in.is_open()) {
            in.seekg(0, std::ios::end);
            const std::streamsize file_size = in.tellg();
            in.seekg(0, std::ios::beg);
            _bytes.resize(static_cast<size_t>(file_size));
            if (file_size > 0) {
                in.read(reinterpret_cast<char *>(_bytes.data()), file_size);
            }
        }
    }
}


uint16_t ClassFileReconstituter::symbolToConstantPoolIndex(std::string sym) const {
    // Резолвим прямо по живому constant pool: индекс CONSTANT_Utf8 == позиция в CP.
    // Хеш-карты по Symbol* ненадёжны (указатель ik->name()/_name не всегда равен
    // указателю Utf8-символа в пуле), поэтому сравниваем по строке.
    ConstantPool *cp = _ik->constants();
    for (int i = 1; i < cp->length(); i++) {
        u1 tag = cp->tags()->at(i);
        if (tag == JVM_CONSTANT_Utf8 && cp->symbol_at(i)->as_string() == sym)
            return static_cast<uint16_t>(i);
        if (tag == JVM_CONSTANT_Long || tag == JVM_CONSTANT_Double)
            i++; // long/double занимают два слота
    }
    return 0;
}

uint16_t ClassFileReconstituter::classNameToConstantPoolIndex(std::string name) const {
    // Находим CONSTANT_Class с нужным именем и возвращаем его индекс в CP
    // (this_class/super_class/interfaces должны ссылаться именно на CONSTANT_Class).
    ConstantPool *cp = _ik->constants();
    for (int i = 1; i < cp->length(); i++) {
        u1 tag = cp->tags()->at(i);
        if ((tag == JVM_CONSTANT_Class || tag == JVM_CONSTANT_UnresolvedClass ||
             tag == JVM_CONSTANT_UnresolvedClassInError) &&
            cp->klass_name_at(i)->as_string() == name)
            return static_cast<uint16_t>(i);
        if (tag == JVM_CONSTANT_Long || tag == JVM_CONSTANT_Double)
            i++;
    }
    return 0;
}

void ClassFileReconstituter::write_utf8(Symbol *symbol) {
    write_u2(symbol->length());
    for (size_t i = 0; i < symbol->length(); i++) {
        write_u1(symbol->byte_at(i));
    }
}

void ClassFileReconstituter::write_u1(uint8_t val) {
    this->stream << val;
}

void ClassFileReconstituter::write_u2(uint16_t val) {
    for (int i = 1; i >= 0; i--)
        this->stream << static_cast<uint8_t>(val >> (i << 3) & 0xFF);
}

void ClassFileReconstituter::write_u4(uint32_t val) {
    for (int i = 3; i >= 0; i--)
        this->stream << static_cast<uint8_t>(val >> (i << 3) & 0xFF);
}

void ClassFileReconstituter::write_u8(uint64_t val) {
    for (int i = 7; i >= 0; i--)
        this->stream << static_cast<uint8_t>(val >> (i << 3) & 0xFF);
}

void ClassFileReconstituter::write_attribute_name_index(const char *name) {
    write_u2(symbolToConstantPoolIndex(std::string(name)));
}

void ClassFileReconstituter::write_method_info(Method *method) {
    AccessFlags access_flags = method->accessFlags();
    ConstMethod *const_method = method->constMethod();
    u2 generic_signature_index = const_method->generic_signature_index();
    u2 method_parameters_count = const_method->has_method_parameters()
                                     ? *const_method->method_parameters_length_addr()
                                     : 0;
    AnnotationArray *anno = method->annotations();
    AnnotationArray *param_anno = method->parameter_annotations();
    AnnotationArray *default_anno = method->annotation_default();
    AnnotationArray *type_anno = method->type_annotations();

    if (method->is_overpass()) {
        return;
    }

    write_u2(access_flags.get_flags() & JVM_RECOGNIZED_METHOD_MODIFIERS);
    write_u2(const_method->name_index());
    write_u2(const_method->signature_index());

    int attr_count = 0;
    if (const_method->code_size() != 0) {
        ++attr_count;
    }

    if (const_method->has_checked_exceptions()) {
        ++attr_count;
    }

    if (default_anno != NULL) {
        ++attr_count;
    }

    if (const_method->has_method_parameters()) {
        ++attr_count;
    }

    if (access_flags.is_synthetic()) {
    }

    if (generic_signature_index != 0) {
        ++attr_count;
    }

    if (anno != NULL) {
        ++attr_count;
    }

    if (param_anno != NULL) {
        ++attr_count;
    }

    if (type_anno != NULL) {
        ++attr_count;
    }

    write_u2(attr_count);
    if (const_method->code_size() > 0) {
        write_code_attribute(method);
    }

    if (const_method->has_checked_exceptions()) {
        write_exceptions_attribute(const_method);
    }

    if (default_anno != NULL) {
        unsigned int length = default_anno->length();
        write_attribute_name_index("AnnotationDefault");
        write_u4(length);
        stream.write(reinterpret_cast<const char *>(default_anno->adr_at(0)), length);
    }

    if (const_method->has_method_parameters()) {
        write_attribute_name_index("MethodParameters");
        write_u4(1 + 4 * method_parameters_count);
        write_u1(method_parameters_count);
        MethodParametersElement *elements = const_method->method_parameters_start();
        for (u2 i = 0; i < method_parameters_count; ++i) {
            write_u2(elements[i].name_cp_index);
            write_u2(elements[i].flags);
        }
    }

    if (access_flags.is_synthetic()) {
    }

    if (generic_signature_index != 0) {
        write_attribute_name_index("Signature");
        write_u4(2);
        write_u2(generic_signature_index);
    }

    if (anno != NULL) {
        unsigned int length = anno->length();
        write_attribute_name_index("RuntimeVisibleAnnotations");
        write_u4(length);
        stream.write(reinterpret_cast<const char *>(anno->adr_at(0)), length);
    }

    if (param_anno != NULL) {
        unsigned int length = param_anno->length();
        write_attribute_name_index("RuntimeVisibleParameterAnnotations");
        write_u4(length);
        stream.write(reinterpret_cast<const char *>(param_anno->adr_at(0)), length);
    }

    if (type_anno != NULL) {
        unsigned int length = type_anno->length();
        write_attribute_name_index("RuntimeVisibleTypeAnnotations");
        write_u4(length);
        stream.write(reinterpret_cast<const char *>(type_anno->adr_at(0)), length);
    }
}

void ClassFileReconstituter::write_code_attribute(Method *method) {
    ConstMethod *const_method = method->constMethod();
    uint16_t line_num_cnt = 0;
    int stackmap_len = 0;
    int local_variable_table_length = 0;
    int local_variable_type_table_length = 0;

    int attr_count = 0;
    int attr_size = 0;
    if (const_method->has_linenumber_table()) {
        line_num_cnt = line_number_table_entries(method);
        if (line_num_cnt != 0) {
            ++attr_count;
            attr_size += 2 + 4 + 2 + line_num_cnt * (2 + 2);
        }
    }

    if (method->constMethod()->has_stackmap_table()) {
        stackmap_len = method->constMethod()->_stack_map_data()->length();
        if (stackmap_len != 0) {
            ++attr_count;
            attr_size += 2 + 4 + stackmap_len;
        }
    }

    if (method->constMethod()->has_localvariable_table()) {
        local_variable_table_length = method->constMethod()->localvariable_table_length();
        if (local_variable_table_length != 0) {
            ++attr_count;
            attr_size += 2 + 4 + 2 + local_variable_table_length * (2 + 2 + 2 + 2 + 2);

            LocalVariableTableElement *elem = method->constMethod()->localvariable_table_start();
            for (int idx = 0; idx < local_variable_table_length; idx++) {
                if (elem[idx].signature_cp_index != 0) {
                    local_variable_type_table_length++;
                }
            }

            if (local_variable_type_table_length != 0) {
                ++attr_count;
                attr_size += 2 + 4 + 2 + local_variable_type_table_length * (2 + 2 + 2 + 2 + 2);
            }
        }
    }

    ExceptionTable exception_table(method);
    int exception_table_length = exception_table.length();

    // Reconstruct the bytecodes before sizing the attribute so code_length always
    // matches what we actually emit (every supported unrewrite preserves length).
    BytecodeRewriter::UnrewriteResult ur = BytecodeRewriter::unrewrite(method);
    if (!ur.ok) {
        fprintf(stderr,
                "[xdAntiDumpMem] ClassFileReconstituter: unknown internal opcode 0x%02X at bci %zu, "
                "emitting raw code (reconstructed bytecode may be invalid for this method)\n",
                ur.fail_opcode, ur.fail_bci);
        ur.bytes = const_method->getBytecode();
    }
    const size_t code_size = ur.bytes.size();
    int size =
            2 + 2 + 4 +
            code_size +
            2 +
            (2 + 2 + 2 + 2) * exception_table_length +
            2 +
            attr_size;

    write_attribute_name_index("Code");
    write_u4(size);
    write_u2(method->constMethod()->_max_stack());
    write_u2(method->constMethod()->_max_locals());
    write_u4(code_size);
    stream.write(reinterpret_cast<const char *>(ur.bytes.data()), code_size);
    write_u2(exception_table_length);
    for (int index = 0; index < exception_table_length; index++) {
        write_u2(exception_table.start_pc(index));
        write_u2(exception_table.end_pc(index));
        write_u2(exception_table.handler_pc(index));
        write_u2(exception_table.catch_type_index(index));
    }
    write_u2(attr_count);
    if (line_num_cnt != 0) {
        write_attribute_name_index("LineNumberTable");
        write_u4(2 + line_num_cnt * (2 + 2));
        write_u2(line_num_cnt);

        CompressedLineNumberReadStream stream(method->constMethod()->compressed_linenumber_table());

        while (stream.read_pair()) {
            write_u2(stream.bci());
            write_u2(stream.line());
        }
    }
    if (stackmap_len != 0) {
        write_attribute_name_index("StackMapTable");
        write_u4(stackmap_len);
        stream.write(reinterpret_cast<const char *>(method->constMethod()->_stack_map_data()->adr_at(0)), stackmap_len);
    }
    if (local_variable_table_length != 0) {
        write_attribute_name_index("LocalVariableTable");
        write_u4(2 + local_variable_table_length * (2 + 2 + 2 + 2 + 2));
        write_u2(local_variable_table_length);

        LocalVariableTableElement *elem = method->constMethod()->localvariable_table_start();
        for (int j = 0; j < method->constMethod()->localvariable_table_length(); j++) {
            write_u2(elem->start_bci);
            write_u2(elem->length);
            write_u2(elem->name_cp_index);
            write_u2(elem->descriptor_cp_index);
            write_u2(elem->slot);
            elem++;
        }
    }
    if (local_variable_type_table_length != 0) {
        write_attribute_name_index("LocalVariableTypeTable");
        write_u4(2 + local_variable_type_table_length * (2 + 2 + 2 + 2 + 2));
        write_u2(local_variable_type_table_length);

        LocalVariableTableElement *elem = method->constMethod()->localvariable_table_start();
        for (int j = 0; j < method->constMethod()->localvariable_table_length(); j++) {
            if (elem->signature_cp_index > 0) {
                write_u2(elem->start_bci);
                write_u2(elem->length);
                write_u2(elem->name_cp_index);
                write_u2(elem->signature_cp_index);
                write_u2(elem->slot);
                local_variable_type_table_length--;
            }
            elem++;
        }
    }
}

void ClassFileReconstituter::write_exceptions_attribute(ConstMethod *const_method) {
    CheckedExceptionElement *checked_exceptions = const_method->checked_exceptions_start();
    int checked_exceptions_length = const_method->checked_exceptions_length();
    int size =
            2 +
            2 * checked_exceptions_length;

    write_attribute_name_index("Exceptions");
    write_u4(size);
    write_u2(checked_exceptions_length);
    for (int index = 0; index < checked_exceptions_length; index++) {
        write_u2(checked_exceptions[index].class_cp_index);
    }
}

void ClassFileReconstituter::write_inner_classes_attribute(InstanceKlass *ik) {
    InnerClassesIterator iter(ik);
    int n = iter.length() / InstanceKlass::inner_class_next_offset;
    write_attribute_name_index("InnerClasses");
    write_u4(2 + n * 4 * 2);
    write_u2(n);
    while (!iter.done()) {
        write_u2(iter.inner_class_info_index());
        write_u2(iter.outer_class_info_index());
        write_u2(iter.inner_name_index());
        write_u2(iter.inner_class_access_flags());
        iter.next();
    }
}

void ClassFileReconstituter::write_enclosing_method_attribute(InstanceKlass *ik) {
    write_attribute_name_index("EnclosingMethod");
    write_u4(4);
    write_u2(ik->enclosing_method_class_index());
    write_u2(ik->enclosing_method_method_index());
}

void ClassFileReconstituter::write_source_debug_extension_attribute(InstanceKlass *ik) {
    const char *ext = ik->source_debug_extension();
    int len = 0;
    if (ext != nullptr) {
        while (ext[len] != '\0') len++;
    }
    write_attribute_name_index("SourceDebugExtension");
    write_u4(len);
    for (int i = 0; i < len; i++) {
        write_u1(static_cast<uint8_t>(ext[i]));
    }
}

void ClassFileReconstituter::write_nest_host_attribute(InstanceKlass *ik) {
    write_attribute_name_index("NestHost");
    write_u4(2);
    write_u2(ik->nest_host_index());
}

void ClassFileReconstituter::write_nest_members_attribute(InstanceKlass *ik) {
    Array<u2> *members = ik->nest_members();
    int n = members->length();
    write_attribute_name_index("NestMembers");
    write_u4(2 + n * 2);
    write_u2(n);
    for (int i = 0; i < n; i++) {
        write_u2(members->at(i));
    }
}

void ClassFileReconstituter::write_permitted_subclasses_attribute(InstanceKlass *ik) {
    Array<u2> *subs = ik->permitted_subclasses();
    int n = subs->length();
    write_attribute_name_index("PermittedSubclasses");
    write_u4(2 + n * 2);
    write_u2(n);
    for (int i = 0; i < n; i++) {
        write_u2(subs->at(i));
    }
}

void ClassFileReconstituter::write_record_attribute(InstanceKlass *ik) {
    Array<RecordComponent *> *components = ik->record_components();
    int n = components->length();

    // Compute total attribute length.
    int attr_len = 2; // components_count
    for (int i = 0; i < n; i++) {
        RecordComponent *rc = components->at(i);
        int component_attr_count = 0;
        int component_attr_len = 0;
        if (rc->generic_signature_index() != 0) {
            component_attr_count++;
            component_attr_len += 2 + 4 + 2; // name_index + length + signature_index
        }
        AnnotationArray *anno = rc->annotations();
        if (anno != nullptr && anno->length() > 0) {
            component_attr_count++;
            component_attr_len += 2 + 4 + anno->length();
        }
        AnnotationArray *type_anno = rc->type_annotations();
        if (type_anno != nullptr && type_anno->length() > 0) {
            component_attr_count++;
            component_attr_len += 2 + 4 + type_anno->length();
        }
        attr_len += 2 + 2 + 2 + component_attr_len; // name_index + descriptor_index + attributes_count + attributes
    }

    write_attribute_name_index("Record");
    write_u4(attr_len);
    write_u2(n);
    for (int i = 0; i < n; i++) {
        RecordComponent *rc = components->at(i);
        write_u2(rc->name_index());
        write_u2(rc->descriptor_index());
        int component_attr_count = 0;
        if (rc->generic_signature_index() != 0) component_attr_count++;
        if (rc->annotations() != nullptr && rc->annotations()->length() > 0) component_attr_count++;
        if (rc->type_annotations() != nullptr && rc->type_annotations()->length() > 0) component_attr_count++;
        write_u2(component_attr_count);
        if (rc->generic_signature_index() != 0) {
            write_attribute_name_index("Signature");
            write_u4(2);
            write_u2(rc->generic_signature_index());
        }
        if (rc->annotations() != nullptr && rc->annotations()->length() > 0) {
            unsigned int length = rc->annotations()->length();
            write_attribute_name_index("RuntimeVisibleAnnotations");
            write_u4(length);
            stream.write(reinterpret_cast<const char *>(rc->annotations()->adr_at(0)), length);
        }
        if (rc->type_annotations() != nullptr && rc->type_annotations()->length() > 0) {
            unsigned int length = rc->type_annotations()->length();
            write_attribute_name_index("RuntimeVisibleTypeAnnotations");
            write_u4(length);
            stream.write(reinterpret_cast<const char *>(rc->type_annotations()->adr_at(0)), length);
        }
    }
}

uint16_t ClassFileReconstituter::line_number_table_entries(Method *method) {
    uint16_t num_entries = 0;
    CompressedLineNumberReadStream stream(method->constMethod()->compressed_linenumber_table());
    while (stream.read_pair()) {
        num_entries++;
    }
    return num_entries;
}

uint16_t ClassFileReconstituter::inner_classes_attribute_length(InstanceKlass *ik) {
    InnerClassesIterator iter(ik);
    return iter.length();
}

ClassFileReconstituter::~ClassFileReconstituter() {
    if (stream.is_open()) {
        stream.close();
    }
}
