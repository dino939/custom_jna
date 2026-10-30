//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_INNERCLASSESITERATOR_H
#define XDANTIDUMPMEM_INNERCLASSESITERATOR_H
#include "InstanceKlass.h"
#include "../array/Array.h"



class InnerClassesIterator {
private:
    // maybe vtable ( in example code nah 0
    Array<jushort>* _inner_classes;
    int _length;
    int _idx;
public:
    InnerClassesIterator(const InstanceKlass* k) {
        _inner_classes = k->inner_classes();
        if (k->inner_classes() != nullptr) {
            _length = _inner_classes->length();
            if (_length % InstanceKlass::inner_class_next_offset == InstanceKlass::enclosing_method_attribute_size) {
                _length -= InstanceKlass::enclosing_method_attribute_size;
            }
        } else {
            _length = 0;
        }
        _idx = 0;
    }

    int length() const {
        return _length;
    }

    bool done() const {
        return _idx >= _length;
    }

    void next() {
        _idx += InstanceKlass::inner_class_next_offset;
    }

    u2 inner_class_info_index() const {
        return _inner_classes->at(_idx + InstanceKlass::inner_class_inner_class_info_offset);
    }

    u2 outer_class_info_index() const {
        return _inner_classes->at(_idx + InstanceKlass::inner_class_outer_class_info_offset);
    }

    u2 inner_name_index() const {
        return _inner_classes->at(_idx + InstanceKlass::inner_class_inner_name_offset);
    }

    u2 inner_class_access_flags() const {
        return _inner_classes->at(_idx + InstanceKlass::inner_class_access_flags_offset);
    }
};


#endif //XDANTIDUMPMEM_INNERCLASSESITERATOR_H
